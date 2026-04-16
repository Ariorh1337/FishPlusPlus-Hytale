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
#include "sdk/BaseDataTypes/Array.h"
#include "sdk/BaseDataTypes/HytaleString.h"
#include <fstream>

#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <optional>
#include <vector>

uint64_t g_RhpNewArrayAddress = 0;

using namespace PacketTable;

// ─────────────────────────────────────────────────────────────────────────────
// JSON value type — supports nesting via recursive variant
// ─────────────────────────────────────────────────────────────────────────────

struct JsonVal;
using JsonObj = std::unordered_map<std::string, JsonVal>;
using JsonArr = std::vector<JsonVal>;

struct JsonVal {
    enum class Kind { Str, Int, Float, Bool, Obj, Arr } kind;
    std::string      s;
    int64_t          i = 0;
    double           f = 0.0;
    bool             b = false;
    JsonObj          obj;
    JsonArr          arr;

    static JsonVal fromStr  (std::string v)  { JsonVal r; r.kind=Kind::Str;   r.s=v;   return r; }
    static JsonVal fromInt  (int64_t v)      { JsonVal r; r.kind=Kind::Int;   r.i=v;   return r; }
    static JsonVal fromFloat(double v)       { JsonVal r; r.kind=Kind::Float; r.f=v;   return r; }
    static JsonVal fromBool (bool v)         { JsonVal r; r.kind=Kind::Bool;  r.b=v;   return r; }
    static JsonVal fromObj  (JsonObj v)      { JsonVal r; r.kind=Kind::Obj;   r.obj=v; return r; }
    static JsonVal fromArr  (JsonArr v)      { JsonVal r; r.kind=Kind::Arr;   r.arr=v; return r; }
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
static std::optional<JsonArr> parseArr(const std::string& s, size_t& i);

static std::optional<JsonVal> parseVal(const std::string& s, size_t& i) {
    skipWs(s, i);
    if (i >= s.size()) return {};

    if (s[i] == '{') {
        auto obj = parseObj(s, i);
        return obj ? std::optional<JsonVal>(JsonVal::fromObj(*obj)) : std::nullopt;
    }
    if (s[i] == '[') {
        auto arr = parseArr(s, i);
        return arr ? std::optional<JsonVal>(JsonVal::fromArr(*arr)) : std::nullopt;
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

static std::optional<JsonArr> parseArr(const std::string& s, size_t& i) {
    skipWs(s, i);
    if (i >= s.size() || s[i] != '[') return {};
    ++i;
    JsonArr out;
    while (i < s.size()) {
        skipWs(s, i);
        if (s[i] == ']') { ++i; break; }
        if (s[i] == ',') { ++i; continue; }
        auto v = parseVal(s, i); 
        if (v) out.push_back(*v);
        else break;
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
        std::string_view p_type = fd.ptr_type ? fd.ptr_type : "";
        if (p_type.starts_with("Array<")) {
            if (val.kind != JsonVal::Kind::Arr) {
                Util::log("[PacketSender]   WARN: field '%s' needs an array value\n", fd.name);
                return false;
            }
            std::string inner_type = std::string(p_type.substr(6, p_type.size() - 7));
            bool is_ptr = inner_type.ends_with("*");
            if (is_ptr) inner_type.pop_back();

            void* mt = SubTypeRegistry::GetMethodTable(fd.ptr_type);
            if (!mt) mt = SubTypeRegistry::GetMethodTable("Array<int>"); // Fallback
            
            auto* arr = Array<uint64_t>::createArray((int)val.arr.size(), mt);
            if (!arr) return false;

            for (size_t k = 0; k < val.arr.size(); ++k) {
                if (is_ptr && val.arr[k].kind == JsonVal::Kind::Obj) {
                    void* child = buildSubObject(inner_type.c_str(), val.arr[k].obj);
                    if (!child) {
                        Util::log("[PacketSender]   WARN: couldn't allocate sub-type '%s' for array element\n", inner_type.c_str());
                        return false;
                    }
                    arr->list[k] = (uint64_t)child;
                } else if (is_ptr && val.arr[k].kind == JsonVal::Kind::Str) {
                    arr->list[k] = 0;
                } else {
                    arr->list[k] = (uint64_t)toInt(val.arr[k]);
                }
            }
            *(void**)base = arr;
            return true;
        }

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
    g_RhpNewArrayAddress = SM::RhpNewArray_GenericAddress;
    SubTypeRegistry::Initialize();
    Util::log("[PacketSender] Initialized\n");
}


// Static helper to find any field equal to 5498
static void DeepSearchCount(void* base, int maxOff, int target, const char* prefix) {
    __try {
        for (int off = 0; off < maxOff; off += 4) {
            int val = *(int*)((uint8_t*)base + off);
            if (val == target) {
                Util::log("[DumpInt] Value %d found at %s+0x%X\n", target, prefix, off);
            }
            
            // If it's a pointer, maybe look inside? (only every 8 bytes)
            if (off % 8 == 0) {
                void* ptr = *(void**)((uint8_t*)base + off);
                if (Util::IsValidPtr(ptr) && ptr != base) {
                    // Check if it's an array header (count at some offset)
                    for (int headerOff = 0x8; headerOff <= 0x18; headerOff += 8) {
                        int count = *(int*)((uint8_t*)ptr + headerOff);
                        if (count == target) {
                            Util::log("[DumpInt] Array of size %d found via %s+0x%X -> Ptr+0x%X\n", target, prefix, off, headerOff);
                        }
                    }
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static bool FindInteractionArraySafely(void* g_LastGameInstance, void** outModule, void** outArray, int* outCount) {
    // Start with a deep search to debug where the count is
    DeepSearchCount(g_LastGameInstance, 0x2000, 5498, "GI");

    __try {
        for (int off = 0x10; off < 0x2000; off += 8) {
            void* potentialModule = *(void**)((uint8_t*)g_LastGameInstance + off);
            if (!Util::IsValidPtr(potentialModule)) continue;

            // Deep search inside module
            // DeepSearchCount(potentialModule, 0x500, 5498, "MOD");

            for (int off2 = 0x0; off2 < 0x800; off2 += 8) {
                void* potentialArray = *(void**)((uint8_t*)potentialModule + off2);
                if (!Util::IsValidPtr(potentialArray)) continue;

                // Check various possible count offsets (0x8, 0x10, 0x18)
                for (int cOff : {0x08, 0x10, 0x18}) {
                    int count = *(int*)((uint8_t*)potentialArray + cOff);
                    if (count == 5498) {
                        *outModule = potentialModule;
                        *outArray = potentialArray;
                        *outCount = count;
                        return true;
                    }
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    return false;
}

// Static helper to read a single entry safely
static bool GetEntrySafely(void* entry, int* outIndex, HytaleString** outNameStr) {
    __try {
        if (!Util::IsValidPtr(entry)) return false;

        // Names are at 0x08 (verified by user output)
        *outNameStr = *(HytaleString**)((uint8_t*)entry + 0x08);

        // Find a unique ID. 335 was likely a constant field.
        // Let's try to look into the RootInteraction object at 0x18
        void* rootObj = *(void**)((uint8_t*)entry + 0x18);
        if (Util::IsValidPtr(rootObj)) {
            // In RootInteraction class, Index is usually at 0x10 or 0x18
            int id10 = *(int*)((uint8_t*)rootObj + 0x10);
            int id18 = *(int*)((uint8_t*)rootObj + 0x18);
            
            if (id10 > 0 && id10 < 10000 && id10 != 335) {
                *outIndex = id10;
            } else if (id18 > 0 && id18 < 10000) {
                *outIndex = id18;
            }
        }

        // If still not found or stuck on 335, fallback to scanning entry itself but skip the 335 field
        if (*outIndex <= 0 || *outIndex == 335) {
            for (int off = 0x10; off < 0x50; off += 4) {
                int val = *(int*)((uint8_t*)entry + off);
                if (val > 0 && val < 50000 && val != 335) {
                    *outIndex = val;
                    break;
                }
            }
        }

        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}


void PacketSender::DumpInteractions() {
    if (!g_LastGameInstance) {
        Util::log("[DumpInt] Fail: No GameInstance pointer. Join a world!\n");
        return;
    }

    // Verified offsets from our audit
    // GameInstance + 0x150 -> InteractionModule
    // InteractionModule + 0x40 -> RootInteractions Array
    void* module = *(void**)((uint8_t*)g_LastGameInstance + 0x150);
    if (!Util::IsValidPtr(module)) {
        Util::log("[DumpInt] Error: InteractionModule not found at GI+0x150\n");
        return;
    }

    void* arrayPtr = *(void**)((uint8_t*)module + 0x40);
    if (!Util::IsValidPtr(arrayPtr)) {
        Util::log("[DumpInt] Error: RootInteractions array not found at Mod+0x40\n");
        return;
    }

    int count = *(int*)((uint8_t*)arrayPtr + 0x08);
    Util::log("[DumpInt] Found %d root interactions. Starting dump...\n", count);

    // Prepare path
    std::string path = "interactions_dump.txt";
    if (Globals::paths && Util::IsValidPtr(Globals::paths->ClientGameDirectory)) {
        path = Globals::paths->ClientGameDirectory->getString() + "\\interactions_dump.txt";
    }
    Util::log("[DumpInt] Saving to: %s\n", path.c_str());

    std::ofstream file(path);
    if (!file.is_open()) {
        Util::log("[DumpInt] ERROR: Could not open file for writing!\n");
        return;
    }

    file << "Index | RootID | InternalName\n";
    file << "------------------------------\n";

    auto* arr = (Array<void*>*)arrayPtr;
    int dumpedCount = 0;

    for (int i = 0; i < count; ++i) {
        int rootId = 0;
        HytaleString* nameStr = nullptr;

        // Use our safe helper to read entry via SEH
        if (GetEntrySafely(arr->list[i], &rootId, &nameStr)) {
            if (Util::IsValidPtr(nameStr)) {
                std::string name = nameStr->getString();
                file << i << " | " << rootId << " | " << name << "\n";
                dumpedCount++;
            }
        }
    }

    file.close();
    Util::log("[DumpInt] Successfully dumped %d entries.\n", dumpedCount);
}
