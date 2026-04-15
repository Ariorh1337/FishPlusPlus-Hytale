/*
 * Copyright (c) FishPlusPlus.
 *
 * PacketSender — send any C2S packet from chat with full nested JSON support:
 *
 *   Simple:
 *     !send-packet {"name":"ClientPlaceBlock","placed_block_id":5}
 *
 *   Nested (ptr fields):
 *     !send-packet {"name":"ClientMovement","absolute_position":{"x":100,"y":64,"z":100}}
 *     !send-packet {"name":"ClientMovement","body_orientation":{"yaw":1.5,"pitch":0}}
 *
 * Ptr fields are allocated via RhpNewFast using MethodTables cached by
 * SubTypeRegistry (populated automatically from received packets).
 */
#include "PacketSender.h"
#include "PacketFieldTable.h"
#include "SubTypeRegistry.h"
#include "core.h"
#include "Util/Packet.h"
#include "sdk/Packets/PacketRegistry.h"
#include "Hooks/Hooks.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <optional>
#include <vector>

using namespace PacketTable;

// ─────────────────────────────────────────────────────────────────────────────
// JSON value type — supports nesting via recursive variant
// ─────────────────────────────────────────────────────────────────────────────

struct JsonVal;
using JsonObj = std::unordered_map<std::string, JsonVal>;

struct JsonVal {
    enum class Kind { Str, Int, Float, Bool, Obj } kind;
    std::string      s;
    int64_t          i = 0;
    double           f = 0.0;
    bool             b = false;
    JsonObj          obj;

    static JsonVal fromStr  (std::string v)  { JsonVal r; r.kind=Kind::Str;   r.s=v;   return r; }
    static JsonVal fromInt  (int64_t v)      { JsonVal r; r.kind=Kind::Int;   r.i=v;   return r; }
    static JsonVal fromFloat(double v)       { JsonVal r; r.kind=Kind::Float; r.f=v;   return r; }
    static JsonVal fromBool (bool v)         { JsonVal r; r.kind=Kind::Bool;  r.b=v;   return r; }
    static JsonVal fromObj  (JsonObj v)      { JsonVal r; r.kind=Kind::Obj;   r.obj=v; return r; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Recursive JSON parser
// ─────────────────────────────────────────────────────────────────────────────

static void skipWs(const std::string& s, size_t& i) {
    while (i < s.size() && (unsigned char)s[i] <= ' ') ++i;
}

static std::optional<std::string> parseStr(const std::string& s, size_t& i) {
    if (i >= s.size() || s[i] != '"') return {};
    ++i;
    std::string out;
    while (i < s.size() && s[i] != '"') {
        if (s[i] == '\\') { ++i; if (i < s.size()) out += s[i]; }
        else out += s[i];
        ++i;
    }
    if (i < s.size()) ++i;
    return out;
}

static std::optional<JsonObj> parseObj(const std::string& s, size_t& i);

static std::optional<JsonVal> parseVal(const std::string& s, size_t& i) {
    skipWs(s, i);
    if (i >= s.size()) return {};

    if (s[i] == '{') {
        auto obj = parseObj(s, i);
        return obj ? std::optional<JsonVal>(JsonVal::fromObj(*obj)) : std::nullopt;
    }
    if (s[i] == '"') {
        auto str = parseStr(s, i);
        return str ? std::optional<JsonVal>(JsonVal::fromStr(*str)) : std::nullopt;
    }
    if (s.substr(i,4) == "true")  { i+=4; return JsonVal::fromBool(true);  }
    if (s.substr(i,5) == "false") { i+=5; return JsonVal::fromBool(false); }
    if (s.substr(i,4) == "null")  { i+=4; return {};                       }

    // Number
    size_t start = i;
    bool isF = false;
    if (s[i] == '-') ++i;
    while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
    if (i < s.size() && s[i] == '.') { isF = true; ++i; }
    while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
    std::string n = s.substr(start, i - start);
    if (n.empty()) return {};
    return isF ? JsonVal::fromFloat(std::stod(n))
               : JsonVal::fromInt((int64_t)std::stoll(n));
}

static std::optional<JsonObj> parseObj(const std::string& s, size_t& i) {
    skipWs(s, i);
    if (i >= s.size() || s[i] != '{') return {};
    ++i;
    JsonObj out;
    while (i < s.size()) {
        skipWs(s, i);
        if (s[i] == '}') { ++i; break; }
        if (s[i] == ',') { ++i; continue; }
        auto k = parseStr(s, i); if (!k) break;
        skipWs(s, i); if (i >= s.size() || s[i] != ':') break; ++i;
        auto v = parseVal(s, i); if (v) out[*k] = *v;
    }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// Field writing
// ─────────────────────────────────────────────────────────────────────────────

static int64_t toInt(const JsonVal& v) {
    if (v.kind == JsonVal::Kind::Int)   return v.i;
    if (v.kind == JsonVal::Kind::Float) return (int64_t)v.f;
    if (v.kind == JsonVal::Kind::Bool)  return v.b ? 1 : 0;
    return 0;
}
static double toDouble(const JsonVal& v) {
    if (v.kind == JsonVal::Kind::Float) return v.f;
    if (v.kind == JsonVal::Kind::Int)   return (double)v.i;
    if (v.kind == JsonVal::Kind::Bool)  return v.b ? 1.0 : 0.0;
    return 0.0;
}

// Forward declaration for recursion
static void* buildSubObject(const char* type_name, const JsonObj& json);

static bool writeField(void* base_ptr, const FieldDesc& fd, const JsonVal& val) {
    auto* base = (uint8_t*)base_ptr + fd.offset;

    if (fd.type == FType::Ptr) {
        // Nested object
        if (val.kind != JsonVal::Kind::Obj || !fd.ptr_type) {
            Util::log("[PacketSender]   WARN: field '%s' needs a JSON object value\n", fd.name);
            return false;
        }
        void* child = buildSubObject(fd.ptr_type, val.obj);
        if (!child) {
            Util::log("[PacketSender]   WARN: couldn't allocate sub-type '%s' "
                      "(MethodTable not cached yet — wait for relevant S2C packet)\n", fd.ptr_type);
            return false;
        }
        *(void**)base = child;
        return true;
    }

    switch (fd.type) {
        case FType::Int32:   *(int32_t*) base = (int32_t) toInt(val);    break;
        case FType::Int64:   *(int64_t*) base =           toInt(val);    break;
        case FType::Float32: *(float*)   base = (float)   toDouble(val); break;
        case FType::Float64: *(double*)  base =           toDouble(val); break;
        case FType::Bool:    *(bool*)    base = (bool)    toInt(val);    break;
        case FType::Int16:   *(int16_t*) base = (int16_t) toInt(val);    break;
        case FType::Int8:    *(int8_t*)  base = (int8_t)  toInt(val);    break;
        default: break;
    }
    return true;
}

// Allocate and fill a sub-type from a JSON object
static void* buildSubObject(const char* type_name, const JsonObj& json) {
    void* obj = SubTypeRegistry::Alloc(type_name);
    if (!obj) return nullptr;

    // Find the SubTypeDef
    const SubTypeDef* def = nullptr;
    for (int i = 0; i < SUB_TYPE_TABLE_SIZE; ++i) {
        if (std::string_view(SUB_TYPE_TABLE[i].name) == type_name) {
            def = &SUB_TYPE_TABLE[i]; break;
        }
    }
    if (!def) return obj;  // no field info, return zeroed object

    // Build a field lookup map
    std::unordered_map<std::string_view, const FieldDesc*> fieldMap;
    for (int i = 0; i < def->field_count; ++i)
        fieldMap[def->fields[i].name] = &def->fields[i];

    for (const auto& [key, val] : json) {
        auto it = fieldMap.find(key);
        if (it == fieldMap.end()) {
            Util::log("[PacketSender]     WARN: '%s' has no field '%s'\n", type_name, key.c_str());
            continue;
        }
        writeField(obj, *it->second, val);
    }
    return obj;
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

bool PacketSender::TrySend(const std::string& json) {
    size_t i = 0;
    auto parsed = parseObj(json, i);
    if (!parsed) {
        Util::log("[PacketSender] Invalid JSON\n");
        return false;
    }

    auto nameIt = parsed->find("name");
    if (nameIt == parsed->end() || nameIt->second.kind != JsonVal::Kind::Str) {
        Util::log("[PacketSender] Missing \"name\" string\n");
        return false;
    }
    const std::string& name = nameIt->second.s;

    // Find packet definition
    const PacketDef* def = nullptr;
    for (int j = 0; j < PKT_TABLE_SIZE; ++j) {
        if (name == PKT_TABLE[j].name) { def = &PKT_TABLE[j]; break; }
    }
    if (!def) {
        Util::log("[PacketSender] Unknown packet: %s\n", name.c_str());
        return false;
    }

    void* pkt = CreatePacket<void*>(def->index);
    if (!pkt) { Util::log("[PacketSender] CreatePacket failed\n"); return false; }

    // Build field lookup
    std::unordered_map<std::string_view, const FieldDesc*> fieldMap;
    for (int j = 0; j < def->field_count; ++j)
        fieldMap[def->fields[j].name] = &def->fields[j];

    int written = 0;
    for (const auto& [key, val] : *parsed) {
        if (key == "name") continue;
        auto it = fieldMap.find(key);
        if (it == fieldMap.end()) {
            Util::log("[PacketSender] Unknown field '%s' on %s\n", key.c_str(), name.c_str());
            continue;
        }
        if (writeField(pkt, *it->second, val)) ++written;
    }

    Packets::SendPacketImmediate(pkt);
    Util::log("[PacketSender] Sent %s  fields_set=%d  sub_types_cached=%d\n",
              name.c_str(), written, SubTypeRegistry::CachedCount());
    return true;
}

bool PacketSender::TryReceive(const std::string& json) {
    size_t i = 0;
    auto parsed = parseObj(json, i);
    if (!parsed) {
        Util::log("[PacketReceiver] Invalid JSON\n");
        return false;
    }

    auto nameIt = parsed->find("name");
    if (nameIt == parsed->end() || nameIt->second.kind != JsonVal::Kind::Str) {
        Util::log("[PacketReceiver] Missing \"name\" string\n");
        return false;
    }
    const std::string& name = nameIt->second.s;

    const PacketDef* def = nullptr;
    for (int j = 0; j < PKT_TABLE_SIZE; ++j) {
        if (name == PKT_TABLE[j].name) { def = &PKT_TABLE[j]; break; }
    }
    if (!def) {
        Util::log("[PacketReceiver] Unknown packet: %s\n", name.c_str());
        return false;
    }

    void* pkt = CreatePacket<void*>(def->index);
    if (!pkt) { Util::log("[PacketReceiver] CreatePacket failed\n"); return false; }

    std::unordered_map<std::string_view, const FieldDesc*> fieldMap;
    for (int j = 0; j < def->field_count; ++j)
        fieldMap[def->fields[j].name] = &def->fields[j];

    int written = 0;
    for (const auto& [key, val] : *parsed) {
        if (key == "name") continue;
        auto it = fieldMap.find(key);
        if (it == fieldMap.end()) {
            Util::log("[PacketReceiver] Unknown field '%s' on %s\n", key.c_str(), name.c_str());
            continue;
        }
        if (writeField(pkt, *it->second, val)) ++written;
    }

    if (!Hooks::g_LastProcessPacketInstance) {
        Util::log("[PacketReceiver] Cannot receive %s: g_LastProcessPacketInstance is null. Wait for at least one natural incoming packet first.\n", name.c_str());
        return false;
    }

    Hooks::hkProcessPacket(Hooks::g_LastProcessPacketInstance, (Object*)pkt);
    Util::log("[PacketReceiver] Received %s  fields_set=%d\n", name.c_str(), written);
    return true;
}

bool PacketSender::CanExecute() { return Util::isFullyInitialized(); }
void PacketSender::Initialize() {
    SubTypeRegistry::Initialize();
    Util::log("[PacketSender] Initialized\n");
}
