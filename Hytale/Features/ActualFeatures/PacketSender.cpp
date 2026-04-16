/*
 * Copyright (c) FishPlusPlus.
 *
 * PacketSender — send any C2S packet from chat with full nested JSON support.
 * Also includes the Packet Lab (Win32 UI) and Dynamic Interaction Name Resolution.
 */
#define NOMINMAX
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
#include <algorithm>

uint64_t g_RhpNewArrayAddress = 0;
using namespace PacketTable;

// ─────────────────────────────────────────────────────────────────────────────
// JSON value type and Recursive Parser
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
    if (s[i] == '{') { auto obj = parseObj(s, i); return obj ? std::optional<JsonVal>(JsonVal::fromObj(*obj)) : std::nullopt; }
    if (s[i] == '[') { auto arr = parseArr(s, i); return arr ? std::optional<JsonVal>(JsonVal::fromArr(*arr)) : std::nullopt; }
    if (s[i] == '"') { auto str = parseStr(s, i); return str ? std::optional<JsonVal>(JsonVal::fromStr(*str)) : std::nullopt; }
    if (s.substr(i,4) == "true")  { i+=4; return JsonVal::fromBool(true);  }
    if (s.substr(i,5) == "false") { i+=5; return JsonVal::fromBool(false); }
    if (s.substr(i,4) == "null")  { i+=4; return {}; }
    size_t start = i; bool isF = false;
    if (s[i] == '-') ++i;
    while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
    if (i < s.size() && s[i] == '.') { isF = true; ++i; }
    while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
    std::string n = s.substr(start, i - start);
    if (n.empty()) return {};
    return isF ? JsonVal::fromFloat(std::stod(n)) : JsonVal::fromInt((int64_t)std::stoll(n));
}

static std::optional<JsonObj> parseObj(const std::string& s, size_t& i) {
    skipWs(s, i); if (i >= s.size() || s[i] != '{') return {}; ++i;
    JsonObj out;
    while (i < s.size()) {
        skipWs(s, i); if (s[i] == '}') { ++i; break; } if (s[i] == ',') { ++i; continue; }
        auto k = parseStr(s, i); if (!k) break;
        skipWs(s, i); if (i >= s.size() || s[i] != ':') break; ++i;
        auto v = parseVal(s, i); if (v) out[*k] = *v;
    }
    return out;
}

static std::optional<JsonArr> parseArr(const std::string& s, size_t& i) {
    skipWs(s, i); if (i >= s.size() || s[i] != '[') return {}; ++i;
    JsonArr out;
    while (i < s.size()) {
        skipWs(s, i); if (s[i] == ']') { ++i; break; } if (s[i] == ',') { ++i; continue; }
        auto v = parseVal(s, i); if (v) out.push_back(*v); else break;
    }
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
// Type Conversion and Field Writing
// ─────────────────────────────────────────────────────────────────────────────

static int64_t toInt(const JsonVal& v) {
    if (v.kind == JsonVal::Kind::Int) return v.i;
    if (v.kind == JsonVal::Kind::Float) return (int64_t)v.f;
    if (v.kind == JsonVal::Kind::Bool) return v.b ? 1 : 0;
    return 0;
}
static double toDouble(const JsonVal& v) {
    if (v.kind == JsonVal::Kind::Float) return v.f;
    if (v.kind == JsonVal::Kind::Int) return (double)v.i;
    if (v.kind == JsonVal::Kind::Bool) return v.b ? 1.0 : 0.0;
    return 0.0;
}

static void* findHytaleString(const std::string& text) {
    if (text.empty()) return nullptr;
    if (!Util::isFullyInitialized() || !g_LastGameInstance) return nullptr;

    // 1. Search in InteractionModule
    void* int_module = *(void**)((uint8_t*)g_LastGameInstance + 0x150);
    if (Util::IsValidPtr(int_module)) {
        void* arrayPtr = *(void**)((uint8_t*)int_module + 0x40);
        if (Util::IsValidPtr(arrayPtr)) {
            int count = *(int*)((uint8_t*)arrayPtr + 0x08);
            Array<void*>* arr = (Array<void*>*)arrayPtr;
            for (int i = 0; i < count; ++i) {
                void* entry = arr->list[i]; if (!Util::IsValidPtr(entry)) continue;
                HytaleString* nameStr = *(HytaleString**)((uint8_t*)entry + 0x08);
                if (Util::IsValidPtr(nameStr) && nameStr->getString() == text) return nameStr;
            }
        }
    }

    // 2. Search in MapModule (Block Names)
    void* map_module = *(void**)((uint8_t*)g_LastGameInstance + 0x118);
    if (Util::IsValidPtr(map_module)) {
        void* blockTypesPtr = *(void**)((uint8_t*)map_module + 0x18);
        if (Util::IsValidPtr(blockTypesPtr)) {
            int count = *(int*)((uint8_t*)blockTypesPtr + 0x08);
            Array<void*>* arr = (Array<void*>*)blockTypesPtr;
            for (int i = 0; i < count; ++i) {
                void* entry = arr->list[i]; if (!Util::IsValidPtr(entry)) continue;
                HytaleString* nameStr = *(HytaleString**)((uint8_t*)entry + 0x08); // ClientBlockType.Name is at 0x08
                if (Util::IsValidPtr(nameStr) && nameStr->getString() == text) return nameStr;
            }
        }
    }

    return nullptr;
}

static void* buildSubObject(const char* type_name, const JsonObj& json);

static const SubTypeDef* findSubTypeDef(const char* name) {
    if (!name) return nullptr;
    for (int i = 0; i < SUB_TYPE_TABLE_SIZE; ++i) {
        if (std::string(SUB_TYPE_TABLE[i].name) == name) return &SUB_TYPE_TABLE[i];
    }
    return nullptr;
}

static bool writeField(void* base_ptr, const FieldDesc& fd, const JsonVal& val);

static void writeFieldsToObj(void* obj, const SubTypeDef* def, const JsonObj& json) {
    std::unordered_map<std::string_view, const FieldDesc*> fieldMap;
    for (int i = 0; i < def->field_count; ++i) fieldMap[def->fields[i].name] = &def->fields[i];
    for (const auto& [key, val] : json) {
        auto it = fieldMap.find(key);
        if (it != fieldMap.end()) writeField(obj, *it->second, val);
    }
}

static bool writeField(void* base_ptr, const FieldDesc& fd, const JsonVal& val) {
    auto* base = (uint8_t*)base_ptr + fd.offset;
    if (fd.type == FType::Ptr) {
        std::string_view p_type = fd.ptr_type ? fd.ptr_type : "";
        if (p_type.starts_with("Array<")) {
            if (val.kind != JsonVal::Kind::Arr) return false;
            std::string inner_type = std::string(p_type.substr(6, p_type.size() - 7));
            bool is_ptr = inner_type.ends_with("*"); if (is_ptr) inner_type.pop_back();
            void* mt = SubTypeRegistry::GetMethodTable(fd.ptr_type);
            if (!mt) mt = SubTypeRegistry::GetMethodTable("Array<int>");
            auto* arr = Array<uint64_t>::createArray((int)val.arr.size(), mt); if (!arr) return false;
            for (size_t k = 0; k < val.arr.size(); ++k) {
                if (is_ptr && val.arr[k].kind == JsonVal::Kind::Obj) arr->list[k] = (uint64_t)buildSubObject(inner_type.c_str(), val.arr[k].obj);
                else arr->list[k] = (uint64_t)toInt(val.arr[k]);
            }
            *(void**)base = arr; return true;
        }
        if (val.kind == JsonVal::Kind::Str && (p_type == "String" || p_type == "HytaleString")) {
            void* s = findHytaleString(val.s);
            if (s) { 
                *(void**)base = s; 
                return true; 
            } else {
                Util::log("[StringLookup] WARNING: String '%s' not found in game memory. Packet field will be null.\n", val.s.c_str());
                return false;
            }
        }
        if (val.kind != JsonVal::Kind::Obj || !fd.ptr_type) return false;
        void* child = buildSubObject(fd.ptr_type, val.obj); if (!child) return false;
        *(void**)base = child; return true;
    }
    if (fd.type == FType::Obj) {
        if (val.kind != JsonVal::Kind::Obj || !fd.ptr_type) return false;
        const SubTypeDef* def = findSubTypeDef(fd.ptr_type);
        if (def) writeFieldsToObj(base, def, val.obj);
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

static void* buildSubObject(const char* type_name, const JsonObj& json) {
    void* obj = SubTypeRegistry::Alloc(type_name); if (!obj) return nullptr;
    const SubTypeDef* def = findSubTypeDef(type_name);
    if (!def) return obj;
    writeFieldsToObj(obj, def, json);
    return obj;
}

// ─────────────────────────────────────────────────────────────────────────────
// Dynamic Interaction Resolution
// ─────────────────────────────────────────────────────────────────────────────

int PacketSender::ResolveInteractionId(const std::string& name) {
    if (!g_LastGameInstance) return -1;
    void* module = *(void**)((uint8_t*)g_LastGameInstance + 0x150);
    if (!Util::IsValidPtr(module)) return -1;
    void* arrayPtr = *(void**)((uint8_t*)module + 0x40);
    if (!Util::IsValidPtr(arrayPtr)) return -1;
    int count = *(int*)((uint8_t*)arrayPtr + 0x08);
    auto* arr = (Array<void*>*)arrayPtr;
    for (int i = 0; i < count; ++i) {
        void* entry = arr->list[i]; if (!Util::IsValidPtr(entry)) continue;
        HytaleString* nameStr = *(HytaleString**)((uint8_t*)entry + 0x08);
        if (Util::IsValidPtr(nameStr) && nameStr->getString() == name) return i;
    }
    return -1;
}

void PacketSender::ResolveNamesInJson(JsonVal& val) {
    if (val.kind == JsonVal::Kind::Obj) {
        for (auto& [key, v] : val.obj) {
            if (v.kind == JsonVal::Kind::Str && v.s.starts_with("INTERACTION~")) {
                std::string name = v.s.substr(12); // Length of "INTERACTION~"
                int id = ResolveInteractionId(name);
                if (id != -1) {
                    v.kind = JsonVal::Kind::Int;
                    v.i = id;
                    Util::log("[PacketLab] Resolved '%s' -> %d\n", name.c_str(), id);
                } else {
                    Util::log("[PacketLab] FAILED to resolve '%s'. Check dump for exact name!\n", name.c_str());
                }
            }
            ResolveNamesInJson(v);
        }
    } else if (val.kind == JsonVal::Kind::Arr) {
        for (auto& v : val.arr) ResolveNamesInJson(v);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

bool PacketSender::TrySend(const std::string& json) {
    size_t i = 0; auto parsed = parseObj(json, i); if (!parsed) return false;
    JsonVal root = JsonVal::fromObj(*parsed); ResolveNamesInJson(root); *parsed = root.obj;
    auto nameIt = parsed->find("name"); if (nameIt == parsed->end()) return false;
    const std::string& name = nameIt->second.s;
    const PacketDef* def = nullptr;
    for (int j = 0; j < PKT_TABLE_SIZE; ++j) { if (name == PKT_TABLE[j].name) { def = &PKT_TABLE[j]; break; } }
    if (!def) return false;
    void* pkt = CreatePacket<void*>(def->index); if (!pkt) return false;
    std::unordered_map<std::string_view, const FieldDesc*> fieldMap;
    for (int j = 0; j < def->field_count; ++j) fieldMap[def->fields[j].name] = &def->fields[j];
    for (const auto& [key, val] : *parsed) {
        if (key == "name") continue;
        auto it = fieldMap.find(key); if (it != fieldMap.end()) writeField(pkt, *it->second, val);
    }
    if (json.find("\"dump\":true") != std::string::npos) {
        Util::log("[PacketSender] DEBUG DUMP of %s:\n", name.c_str());
        Util::HexDump(pkt, 0x100); // Dump first 256 bytes
        if (def->index == SyncInteractionChains_BI) {
             void* updates = *(void**)((uint8_t*)pkt + 0x08);
             if (Util::IsValidPtr(updates)) {
                 int count = *(int*)((uint8_t*)updates + 0x08);
                 Util::log("[PacketSender] updates Array count: %d\n", count);
                 Array<void*>* arr = (Array<void*>*)updates;
                 for (int k = 0; k < count; ++k) {
                     Util::log("[PacketSender] SyncInteractionChain [%d] at %p:\n", k, arr->list[k]);
                     Util::HexDump(arr->list[k], 0x80);
                 }
             }
        }
    }
    Packets::SendPacketImmediate(pkt);
    Util::log("[PacketSender] Sent %s\n", name.c_str());
    return true;
}

bool PacketSender::TryReceive(const std::string& json) {
    size_t i = 0; auto parsed = parseObj(json, i); if (!parsed) return false;
    auto nameIt = parsed->find("name"); if (nameIt == parsed->end()) return false;
    const std::string& name = nameIt->second.s;
    const PacketDef* def = nullptr;
    for (int j = 0; j < PKT_TABLE_SIZE; ++j) { if (name == PKT_TABLE[j].name) { def = &PKT_TABLE[j]; break; } }
    if (!def) return false;
    void* pkt = CreatePacket<void*>(def->index); if (!pkt) return false;
    std::unordered_map<std::string_view, const FieldDesc*> fieldMap;
    for (int j = 0; j < def->field_count; ++j) fieldMap[def->fields[j].name] = &def->fields[j];
    for (const auto& [key, val] : *parsed) {
        if (key == "name") continue;
        auto it = fieldMap.find(key); if (it != fieldMap.end()) writeField(pkt, *it->second, val);
    }
    if (Hooks::g_LastProcessPacketInstance) Hooks::hkProcessPacket(Hooks::g_LastProcessPacketInstance, (Object*)pkt);
    return true;
}

void PacketSender::DumpInteractions() {
    if (!g_LastGameInstance) return;
    void* module = *(void**)((uint8_t*)g_LastGameInstance + 0x150); if (!Util::IsValidPtr(module)) return;
    void* arrayPtr = *(void**)((uint8_t*)module + 0x40); if (!Util::IsValidPtr(arrayPtr)) return;
    int count = *(int*)((uint8_t*)arrayPtr + 0x08);
    std::string path = (Globals::paths && Util::IsValidPtr(Globals::paths->ClientGameDirectory)) ? Globals::paths->ClientGameDirectory->getString() + "\\interactions_dump.txt" : "interactions_dump.txt";
    std::ofstream file(path); if (!file.is_open()) return;
    file << "ID | InternalName\n-----------------\n";
    auto* arr = (Array<void*>*)arrayPtr;
    for (int i = 0; i < count; ++i) {
        void* entry = arr->list[i]; if (!Util::IsValidPtr(entry)) continue;
        HytaleString* nameStr = *(HytaleString**)((uint8_t*)entry + 0x08);
        if (Util::IsValidPtr(nameStr)) file << i << " | " << nameStr->getString() << "\n";
    }
    file.close(); Util::log("[DumpInt] Success: %s\n", path.c_str());
}

bool PacketSender::CanExecute() { return Util::isFullyInitialized(); }
void PacketSender::Initialize() { g_RhpNewArrayAddress = SM::RhpNewArray_GenericAddress; SubTypeRegistry::Initialize(); }

// ─────────────────────────────────────────────────────────────────────────────
// Win32 Packet Lab UI
// ─────────────────────────────────────────────────────────────────────────────

#define ID_BTN_SEND 101
#define ID_EDIT_JSON 102
static HWND hEdit = NULL;

LRESULT CALLBACK PacketLabWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        hEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, 10, 10, 560, 300, hWnd, (HMENU)ID_EDIT_JSON, NULL, NULL);
        HFONT hFont = CreateFontA(16, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
        SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
        CreateWindowA("BUTTON", "Send Packet", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 320, 120, 30, hWnd, (HMENU)ID_BTN_SEND, NULL, NULL);
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_BTN_SEND) {
            int len = GetWindowTextLengthA(hEdit);
            if (len > 0) { char* buf = new char[len + 1]; GetWindowTextA(hEdit, buf, len + 1); std::string json(buf); delete[] buf; PacketSender::TrySend(json); }
        }
        break;
    }
    case WM_CLOSE: ShowWindow(hWnd, SW_HIDE); return 0;
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void PacketSender::OpenPacketLabUI() {
    static bool registered = false; static HWND hWndLab = NULL;
    if (!registered) {
        WNDCLASSEXA wc = { sizeof(WNDCLASSEXA) }; wc.lpfnWndProc = PacketLabWndProc; wc.hInstance = GetModuleHandleA(NULL); wc.lpszClassName = "PacketLabClass"; wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        RegisterClassExA(&wc); registered = true;
    }
    if (!hWndLab) hWndLab = CreateWindowExA(WS_EX_TOPMOST, "PacketLabClass", "Fish++ Packet Lab", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, GetModuleHandleA(NULL), NULL);
    ShowWindow(hWndLab, SW_SHOW); UpdateWindow(hWndLab); SetForegroundWindow(hWndLab);
}

const char* PacketSender::GetPacketName(int index) {
    for (int i = 0; i < PKT_TABLE_SIZE; ++i) {
        if (PKT_TABLE[i].index == index) return PKT_TABLE[i].name;
    }
    return nullptr;
}
