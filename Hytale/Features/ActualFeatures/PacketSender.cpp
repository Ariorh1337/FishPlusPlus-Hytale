/*
 * Copyright (c) FishPlusPlus.
 */
#include "PacketSender.h"
#include "core.h"
#include "Util/Packet.h"
#include "sdk/Packets/PacketRegistry.h"

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <optional>
#include <filesystem>

// ─────────────────────────────────────────────────────────────────────────────
// Minimal flat JSON parser — handles {"key": value, ...} where value is
// a string, integer, float, or boolean. No nesting needed for chat commands.
// ─────────────────────────────────────────────────────────────────────────────

using JsonValue = std::variant<std::string, int64_t, double, bool>;
using JsonObj   = std::unordered_map<std::string, JsonValue>;

static void skipWs(const std::string& s, size_t& i) {
    while (i < s.size() && std::isspace((unsigned char)s[i])) ++i;
}

static std::optional<std::string> parseString(const std::string& s, size_t& i) {
    if (i >= s.size() || s[i] != '"') return {};
    ++i;
    std::string out;
    while (i < s.size() && s[i] != '"') {
        if (s[i] == '\\') { ++i; if (i < s.size()) out += s[i]; }
        else out += s[i];
        ++i;
    }
    if (i < s.size()) ++i; // skip closing "
    return out;
}

static std::optional<JsonValue> parseValue(const std::string& s, size_t& i) {
    skipWs(s, i);
    if (i >= s.size()) return {};

    // String
    if (s[i] == '"') {
        auto str = parseString(s, i);
        if (!str) return {};
        return JsonValue{ *str };
    }

    // Boolean / null
    if (s.substr(i, 4) == "true")  { i += 4; return JsonValue{ true }; }
    if (s.substr(i, 5) == "false") { i += 5; return JsonValue{ false }; }
    if (s.substr(i, 4) == "null")  { i += 4; return {}; }  // skip nulls

    // Number (int or float)
    size_t start = i;
    bool isFloat = false;
    if (s[i] == '-') ++i;
    while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
    if (i < s.size() && s[i] == '.') { isFloat = true; ++i; }
    while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
    if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
        isFloat = true; ++i;
        if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
        while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
    }

    std::string numStr = s.substr(start, i - start);
    if (numStr.empty()) return {};
    if (isFloat) return JsonValue{ std::stod(numStr) };
    return JsonValue{ (int64_t)std::stoll(numStr) };
}

static std::optional<JsonObj> parseJsonObject(const std::string& s) {
    JsonObj out;
    size_t i = 0;
    skipWs(s, i);
    if (i >= s.size() || s[i] != '{') return {};
    ++i;

    while (true) {
        skipWs(s, i);
        if (i >= s.size()) break;
        if (s[i] == '}') break;
        if (s[i] == ',') { ++i; continue; }

        auto key = parseString(s, i);
        if (!key) break;

        skipWs(s, i);
        if (i >= s.size() || s[i] != ':') break;
        ++i;

        auto val = parseValue(s, i);
        if (val) out[*key] = *val;
    }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// Descriptor loading (from packet_descriptors.json)
// ─────────────────────────────────────────────────────────────────────────────

struct FieldDesc {
    std::string name;
    std::string type;   // int32, int64, float32, float64, bool, int16, int8, enum32
    size_t      offset;
    int         size;
    bool        settable;
};

struct PacketDesc {
    PacketIndex             index;
    int                     id;
    std::string             direction;
    std::vector<FieldDesc>  fields;
};

static std::unordered_map<std::string, PacketDesc> g_descs;
static bool g_loaded = false;

// Tiny JSON parser for the descriptor file (only what we need)
static std::string readJsonString(const std::string& s, size_t& i) {
    auto v = parseString(s, i);
    return v ? *v : "";
}

static int64_t readJsonInt(const std::string& s, size_t& i) {
    auto v = parseValue(s, i);
    if (!v) return 0;
    if (auto* p = std::get_if<int64_t>(&*v)) return *p;
    if (auto* p = std::get_if<double>(&*v))  return (int64_t)*p;
    return 0;
}

// Parse the descriptor JSON into g_descs
// Format: { "PacketName": { "index":N, "id":N, "direction":"C2S",
//            "fields":[{"name":"f","type":"t","offset":N,"size":N,"settable":true},...] } }
static void parseDescriptors(const std::string& src) {
    // We'll do a simple line/token scan rather than a full recursive parser.
    // The file is regular enough for this.

    // Split into per-packet blocks by finding top-level keys
    size_t i = 0;
    skipWs(src, i);
    if (i >= src.size() || src[i] != '{') return;
    ++i;

    while (i < src.size()) {
        skipWs(src, i);
        if (src[i] == '}') break;
        if (src[i] == ',') { ++i; continue; }

        // Packet name
        auto pktName = parseString(src, i);
        if (!pktName) break;
        skipWs(src, i);
        if (i >= src.size() || src[i] != ':') break;
        ++i;
        skipWs(src, i);
        if (i >= src.size() || src[i] != '{') break;
        ++i;

        PacketDesc desc;

        // Find end of this packet block (matching '}')
        while (i < src.size()) {
            skipWs(src, i);
            if (src[i] == '}') { ++i; break; }
            if (src[i] == ',') { ++i; continue; }

            auto key = parseString(src, i);
            if (!key) break;
            skipWs(src, i);
            if (i >= src.size() || src[i] != ':') break;
            ++i;
            skipWs(src, i);

            if (*key == "index") {
                desc.index = (PacketIndex)(int)readJsonInt(src, i);
            } else if (*key == "id") {
                desc.id = (int)readJsonInt(src, i);
            } else if (*key == "direction") {
                desc.direction = readJsonString(src, i);
            } else if (*key == "fields") {
                // Parse array of field objects
                if (i < src.size() && src[i] == '[') {
                    ++i;
                    while (i < src.size()) {
                        skipWs(src, i);
                        if (src[i] == ']') { ++i; break; }
                        if (src[i] == ',') { ++i; continue; }
                        if (src[i] != '{') { ++i; continue; }
                        ++i;

                        FieldDesc fd;
                        while (i < src.size()) {
                            skipWs(src, i);
                            if (src[i] == '}') { ++i; break; }
                            if (src[i] == ',') { ++i; continue; }
                            auto fkey = parseString(src, i);
                            if (!fkey) break;
                            skipWs(src, i);
                            if (i >= src.size() || src[i] != ':') break;
                            ++i;
                            skipWs(src, i);

                            if (*fkey == "name")     fd.name     = readJsonString(src, i);
                            else if (*fkey == "type") fd.type    = readJsonString(src, i);
                            else if (*fkey == "offset") fd.offset = (size_t)readJsonInt(src, i);
                            else if (*fkey == "size") fd.size    = (int)readJsonInt(src, i);
                            else if (*fkey == "settable") {
                                auto v = parseValue(src, i);
                                fd.settable = v && std::holds_alternative<bool>(*v) && std::get<bool>(*v);
                            } else {
                                parseValue(src, i); // skip unknown
                            }
                        }
                        desc.fields.push_back(std::move(fd));
                    }
                }
            } else {
                parseValue(src, i); // skip unknown top-level fields
            }
        }

        g_descs[*pktName] = std::move(desc);
    }
}

static void LoadDescriptors() {
    if (g_loaded) return;
    g_loaded = true;

    // Look for packet_descriptors.json next to the exe
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    std::filesystem::path jsonPath = std::filesystem::path(exePath).parent_path() / "packet_descriptors.json";

    std::ifstream f(jsonPath);
    if (!f.is_open()) {
        Util::log("[PacketSender] Could not open %s\n", jsonPath.string().c_str());
        return;
    }

    std::ostringstream ss;
    ss << f.rdbuf();
    parseDescriptors(ss.str());

    Util::log("[PacketSender] Loaded %zu packet descriptors from %s\n",
        g_descs.size(), jsonPath.string().c_str());
}

// ─────────────────────────────────────────────────────────────────────────────
// Field writing
// ─────────────────────────────────────────────────────────────────────────────

static double toDouble(const JsonValue& v) {
    if (auto* p = std::get_if<double>(&v))  return *p;
    if (auto* p = std::get_if<int64_t>(&v)) return (double)*p;
    if (auto* p = std::get_if<bool>(&v))    return *p ? 1.0 : 0.0;
    return 0.0;
}

static int64_t toInt(const JsonValue& v) {
    if (auto* p = std::get_if<int64_t>(&v)) return *p;
    if (auto* p = std::get_if<double>(&v))  return (int64_t)*p;
    if (auto* p = std::get_if<bool>(&v))    return *p ? 1 : 0;
    return 0;
}

static bool toBool(const JsonValue& v) {
    if (auto* p = std::get_if<bool>(&v))    return *p;
    if (auto* p = std::get_if<int64_t>(&v)) return *p != 0;
    if (auto* p = std::get_if<double>(&v))  return *p != 0.0;
    return false;
}

static void writeField(void* packet, const FieldDesc& fd, const JsonValue& val) {
    uint8_t* base = (uint8_t*)packet;
    uint8_t* ptr  = base + fd.offset;

    if      (fd.type == "int32"  || fd.type == "enum32") *(int32_t*) ptr = (int32_t)toInt(val);
    else if (fd.type == "int64")                          *(int64_t*) ptr = toInt(val);
    else if (fd.type == "float32")                        *(float*)   ptr = (float)toDouble(val);
    else if (fd.type == "float64")                        *(double*)  ptr = toDouble(val);
    else if (fd.type == "bool")                           *(bool*)    ptr = toBool(val);
    else if (fd.type == "int16")                          *(int16_t*) ptr = (int16_t)toInt(val);
    else if (fd.type == "int8")                           *(int8_t*)  ptr = (int8_t)toInt(val);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

bool PacketSender::TrySend(const std::string& json) {
    LoadDescriptors();

    auto obj = parseJsonObject(json);
    if (!obj) {
        Util::log("[PacketSender] Invalid JSON: %s\n", json.c_str());
        return false;
    }

    // Require "name" field
    auto nameIt = obj->find("name");
    if (nameIt == obj->end()) {
        Util::log("[PacketSender] Missing \"name\" field\n");
        return false;
    }
    auto* nameStr = std::get_if<std::string>(&nameIt->second);
    if (!nameStr) {
        Util::log("[PacketSender] \"name\" must be a string\n");
        return false;
    }

    // Look up descriptor
    auto descIt = g_descs.find(*nameStr);
    if (descIt == g_descs.end()) {
        Util::log("[PacketSender] Unknown packet: %s\n", nameStr->c_str());
        return false;
    }
    const PacketDesc& desc = descIt->second;

    // Allocate
    Object* packet = CreatePacket<Object*>(desc.index);
    if (!packet) {
        Util::log("[PacketSender] CreatePacket failed for %s\n", nameStr->c_str());
        return false;
    }

    // Build a field lookup map from the descriptor
    std::unordered_map<std::string, const FieldDesc*> fieldMap;
    for (const auto& fd : desc.fields)
        fieldMap[fd.name] = &fd;

    // Write each provided field
    int written = 0;
    for (const auto& [key, val] : *obj) {
        if (key == "name") continue;

        auto it = fieldMap.find(key);
        if (it == fieldMap.end()) {
            Util::log("[PacketSender]   WARN: unknown field '%s' on %s\n", key.c_str(), nameStr->c_str());
            continue;
        }
        const FieldDesc& fd = *it->second;
        if (!fd.settable) {
            Util::log("[PacketSender]   WARN: field '%s' is not settable (type=%s)\n",
                key.c_str(), fd.type.c_str());
            continue;
        }

        writeField(packet, fd, val);
        Util::log("[PacketSender]   %s = ... (offset=0x%zX type=%s)\n",
            key.c_str(), fd.offset, fd.type.c_str());
        ++written;
    }

    Packets::SendPacketImmediate(packet);
    Util::log("[PacketSender] Sent %s (index=%d id=%d fields_set=%d)\n",
        nameStr->c_str(), (int)desc.index, desc.id, written);

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Feature boilerplate
// ─────────────────────────────────────────────────────────────────────────────

bool PacketSender::CanExecute() {
    return Util::isFullyInitialized();
}

void PacketSender::Initialize() {
    LoadDescriptors();
    Util::log("[PacketSender] Initialized\n");
}
