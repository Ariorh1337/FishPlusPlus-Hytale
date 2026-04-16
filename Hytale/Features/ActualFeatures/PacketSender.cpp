/*
 * Copyright (c) FishPlusPlus.
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

using namespace PacketTable;

// JSON types

struct JsonVal;
using JsonObj = std::unordered_map<std::string, JsonVal>;
using JsonArr = std::vector<JsonVal>;

struct JsonVal {
	enum class Kind { Str, Int, Float, Bool, Obj, Arr } kind;
	std::string s;
	int64_t     i = 0;
	double      f = 0.0;
	bool        b = false;
	JsonObj     obj;
	JsonArr     arr;

	static JsonVal fromStr  (std::string v)  { JsonVal r; r.kind=Kind::Str;   r.s=v;   return r; }
	static JsonVal fromInt  (int64_t v)      { JsonVal r; r.kind=Kind::Int;   r.i=v;   return r; }
	static JsonVal fromFloat(double v)       { JsonVal r; r.kind=Kind::Float; r.f=v;   return r; }
	static JsonVal fromBool (bool v)         { JsonVal r; r.kind=Kind::Bool;  r.b=v;   return r; }
	static JsonVal fromObj  (JsonObj v)      { JsonVal r; r.kind=Kind::Obj;   r.obj=v; return r; }
	static JsonVal fromArr  (JsonArr v)      { JsonVal r; r.kind=Kind::Arr;   r.arr=v; return r; }
};

// JSON parser

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
		auto o = parseObj(s, i);
		return o ? std::optional<JsonVal>(JsonVal::fromObj(*o)) : std::nullopt;
	}
	if (s[i] == '[') {
		auto a = parseArr(s, i);
		return a ? std::optional<JsonVal>(JsonVal::fromArr(*a)) : std::nullopt;
	}
	if (s[i] == '"') {
		auto t = parseStr(s, i);
		return t ? std::optional<JsonVal>(JsonVal::fromStr(*t)) : std::nullopt;
	}
	if (s.substr(i, 4) == "true")  { i += 4; return JsonVal::fromBool(true);  }
	if (s.substr(i, 5) == "false") { i += 5; return JsonVal::fromBool(false); }
	if (s.substr(i, 4) == "null")  { i += 4; return {}; }

	size_t start = i;
	bool isF = false;
	if (s[i] == '-') ++i;
	while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
	if (i < s.size() && s[i] == '.') { isF = true; ++i; }
	while (i < s.size() && std::isdigit((unsigned char)s[i])) ++i;
	std::string n = s.substr(start, i - start);
	if (n.empty()) return {};
	return isF ? JsonVal::fromFloat(std::stod(n)) : JsonVal::fromInt((int64_t)std::stoll(n));
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
		auto k = parseStr(s, i);
		if (!k) break;
		skipWs(s, i);
		if (i >= s.size() || s[i] != ':') break;
		++i;
		auto v = parseVal(s, i);
		if (v) out[*k] = *v;
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

// Type coercion

static int64_t toInt(const JsonVal& v) {
	switch (v.kind) {
		case JsonVal::Kind::Int:   return v.i;
		case JsonVal::Kind::Float: return (int64_t)v.f;
		case JsonVal::Kind::Bool:  return v.b ? 1 : 0;
		default:                   return 0;
	}
}

static double toDouble(const JsonVal& v) {
	switch (v.kind) {
		case JsonVal::Kind::Float: return v.f;
		case JsonVal::Kind::Int:   return (double)v.i;
		case JsonVal::Kind::Bool:  return v.b ? 1.0 : 0.0;
		default:                   return 0.0;
	}
}

// HytaleString lookup
// Searches two module arrays for an existing string pointer matching the given text.
//   GameInstance + 0x150 = InteractionModule -> ClientRootInteraction[] (+ 0x40), each entry Name at + 0x08
//   GameInstance + 0x118 = MapModule         -> ClientBlockType[]       (+ 0x18), each entry Name at + 0x08
static void* findHytaleString(const std::string& text) {
	if (text.empty() || !Util::isFullyInitialized() || !g_LastGameInstance) return nullptr;

	void* int_module = *(void**)((uint8_t*)g_LastGameInstance + 0x150);
	if (Util::IsValidPtr(int_module)) {
		void* arrayPtr = *(void**)((uint8_t*)int_module + 0x40);
		if (Util::IsValidPtr(arrayPtr)) {
			int count = *(int*)((uint8_t*)arrayPtr + 0x08);
			auto* arr = (Array<void*>*)arrayPtr;
			for (int i = 0; i < count; ++i) {
				void* entry = arr->list[i];
				if (!Util::IsValidPtr(entry)) continue;
				HytaleString* nameStr = *(HytaleString**)((uint8_t*)entry + 0x08);
				if (Util::IsValidPtr(nameStr) && nameStr->getString() == text) return nameStr;
			}
		}
	}

	void* map_module = *(void**)((uint8_t*)g_LastGameInstance + 0x118);
	if (Util::IsValidPtr(map_module)) {
		void* blockTypesPtr = *(void**)((uint8_t*)map_module + 0x18);
		if (Util::IsValidPtr(blockTypesPtr)) {
			int count = *(int*)((uint8_t*)blockTypesPtr + 0x08);
			auto* arr = (Array<void*>*)blockTypesPtr;
			for (int i = 0; i < count; ++i) {
				void* entry = arr->list[i];
				if (!Util::IsValidPtr(entry)) continue;
				HytaleString* nameStr = *(HytaleString**)((uint8_t*)entry + 0x08);
				if (Util::IsValidPtr(nameStr) && nameStr->getString() == text) return nameStr;
			}
		}
	}

	return nullptr;
}

// Sub-object construction

static bool writeField(void* base_ptr, const FieldDesc& fd, const JsonVal& val);

static const SubTypeDef* findSubTypeDef(const char* name) {
	if (!name) return nullptr;
	for (int i = 0; i < SUB_TYPE_TABLE_SIZE; ++i)
		if (std::string(SUB_TYPE_TABLE[i].name) == name) return &SUB_TYPE_TABLE[i];
	return nullptr;
}

static void writeFieldsToObj(void* obj, const SubTypeDef* def, const JsonObj& json) {
	std::unordered_map<std::string_view, const FieldDesc*> fieldMap;
	for (int i = 0; i < def->field_count; ++i) fieldMap[def->fields[i].name] = &def->fields[i];
	for (const auto& [key, val] : json) {
		auto it = fieldMap.find(key);
		if (it != fieldMap.end()) writeField(obj, *it->second, val);
	}
}

static void* buildSubObject(const char* type_name, const JsonObj& json) {
	void* obj = SubTypeRegistry::Alloc(type_name);
	if (!obj) {
		Util::log("[PacketSender] MISSING MethodTable for '%s' — not in SubTypeRegistry::Initialize()\n", type_name);
		return nullptr;
	}
	const SubTypeDef* def = findSubTypeDef(type_name);
	if (def) writeFieldsToObj(obj, def, json);
	return obj;
}

static bool writeField(void* base_ptr, const FieldDesc& fd, const JsonVal& val) {
	auto* base = (uint8_t*)base_ptr + fd.offset;

	if (fd.type == FType::Ptr) {
		std::string_view p_type = fd.ptr_type ? fd.ptr_type : "";

		if (p_type.starts_with("Array<")) {
			if (val.kind != JsonVal::Kind::Arr) return false;
			std::string inner_type = std::string(p_type.substr(6, p_type.size() - 7));
			bool is_ptr = inner_type.ends_with("*");
			if (is_ptr) inner_type.pop_back();

			void* mt = SubTypeRegistry::GetMethodTable(fd.ptr_type);
			if (!mt) {
				Util::log("[PacketSender] MISSING array MethodTable for '%s' — add to SubTypeRegistry::Initialize()\n", fd.ptr_type);
				return false;
			}
			auto* arr = Array<uint64_t>::createArray((int)val.arr.size(), mt);
			if (!arr) return false;
			for (size_t k = 0; k < val.arr.size(); ++k) {
				if (is_ptr && val.arr[k].kind == JsonVal::Kind::Obj)
					arr->list[k] = (uint64_t)buildSubObject(inner_type.c_str(), val.arr[k].obj);
				else
					arr->list[k] = (uint64_t)toInt(val.arr[k]);
			}
			*(void**)base = arr;
			return true;
		}

		if (val.kind == JsonVal::Kind::Str && (p_type == "String" || p_type == "HytaleString")) {
			void* s = findHytaleString(val.s);
			if (s) { *(void**)base = s; return true; }
			Util::log("[PacketSender] String '%s' not found in game memory\n", val.s.c_str());
			return false;
		}

		if (val.kind != JsonVal::Kind::Obj || !fd.ptr_type) return false;
		void* child = buildSubObject(fd.ptr_type, val.obj);
		if (!child) return false;
		*(void**)base = child;
		return true;
	}

	if (fd.type == FType::Obj) {
		if (val.kind != JsonVal::Kind::Obj || !fd.ptr_type) return false;
		const SubTypeDef* def = findSubTypeDef(fd.ptr_type);
		if (def) writeFieldsToObj(base, def, val.obj);
		return true;
	}

	switch (fd.type) {
		case FType::Int8:    *(int8_t*) base = (int8_t) toInt(val);    break;
		case FType::Int16:   *(int16_t*)base = (int16_t)toInt(val);    break;
		case FType::Int32:   *(int32_t*)base = (int32_t)toInt(val);    break;
		case FType::Int64:   *(int64_t*)base =          toInt(val);    break;
		case FType::Float32: *(float*)  base = (float)  toDouble(val); break;
		case FType::Float64: *(double*) base =           toDouble(val); break;
		case FType::Bool:    *(bool*)   base = (bool)   toInt(val);    break;
		default: break;
	}
	return true;
}

// Interaction name resolution
// GameInstance + 0x150 = InteractionModule -> ClientRootInteraction[] (+ 0x40), each entry Name at + 0x08

int PacketSender::ResolveInteractionId(const std::string& name) {
	if (!g_LastGameInstance) return -1;
	void* module = *(void**)((uint8_t*)g_LastGameInstance + 0x150);
	if (!Util::IsValidPtr(module)) return -1;
	void* arrayPtr = *(void**)((uint8_t*)module + 0x40);
	if (!Util::IsValidPtr(arrayPtr)) return -1;
	int count = *(int*)((uint8_t*)arrayPtr + 0x08);
	auto* arr = (Array<void*>*)arrayPtr;
	for (int i = 0; i < count; ++i) {
		void* entry = arr->list[i];
		if (!Util::IsValidPtr(entry)) continue;
		HytaleString* nameStr = *(HytaleString**)((uint8_t*)entry + 0x08);
		if (Util::IsValidPtr(nameStr) && nameStr->getString() == name) return i;
	}
	return -1;
}

void PacketSender::ResolveNamesInJson(JsonVal& val) {
	if (val.kind == JsonVal::Kind::Obj) {
		for (auto& [key, v] : val.obj) {
			if (v.kind == JsonVal::Kind::Str && v.s.starts_with("INTERACTION~")) {
				std::string iname = v.s.substr(12);
				int id = ResolveInteractionId(iname);
				if (id != -1) {
					v = JsonVal::fromInt(id);
					Util::log("[PacketLab] Resolved '%s' -> %d\n", iname.c_str(), id);
				} else {
					Util::log("[PacketLab] FAILED to resolve '%s' — use !dump-interactions\n", iname.c_str());
				}
			}
			ResolveNamesInJson(v);
		}
	} else if (val.kind == JsonVal::Kind::Arr) {
		for (auto& v : val.arr) ResolveNamesInJson(v);
	}
}

// Shared build helper — parse JSON, resolve names, fill packet fields

static void* buildPacketFromJson(const std::string& json, const PacketDef*& outDef) {
	size_t i = 0;
	auto parsed = parseObj(json, i);
	if (!parsed) return nullptr;

	JsonVal root = JsonVal::fromObj(*parsed);
	PacketSender::ResolveNamesInJson(root);
	*parsed = root.obj;

	auto nameIt = parsed->find("name");
	if (nameIt == parsed->end() || nameIt->second.kind != JsonVal::Kind::Str) return nullptr;
	const std::string& name = nameIt->second.s;

	outDef = nullptr;
	for (int j = 0; j < PKT_TABLE_SIZE; ++j) {
		if (name == PKT_TABLE[j].name) { outDef = &PKT_TABLE[j]; break; }
	}
	if (!outDef) {
		Util::log("[PacketSender] Unknown packet name '%s'\n", name.c_str());
		return nullptr;
	}

	void* pkt = CreatePacket<void*>(outDef->index);
	if (!pkt) return nullptr;

	std::unordered_map<std::string_view, const FieldDesc*> fieldMap;
	for (int j = 0; j < outDef->field_count; ++j) fieldMap[outDef->fields[j].name] = &outDef->fields[j];

	for (const auto& [key, val] : *parsed) {
		if (key == "name" || key == "dump") continue;
		auto it = fieldMap.find(key);
		if (it != fieldMap.end()) writeField(pkt, *it->second, val);
	}

	if (parsed->count("dump") && parsed->at("dump").kind == JsonVal::Kind::Bool && parsed->at("dump").b) {
		Util::log("[PacketSender] HexDump of %s:\n", outDef->name);
		Util::HexDump(pkt, 0x100);
	}

	return pkt;
}

// Public API

bool PacketSender::TrySend(const std::string& json) {
	const PacketDef* def = nullptr;
	void* pkt = buildPacketFromJson(json, def);
	if (!pkt) return false;
	Packets::SendPacketImmediate(pkt);
	Util::log("[PacketSender] Sent %s\n", def->name);
	return true;
}

bool PacketSender::TryReceive(const std::string& json) {
	const PacketDef* def = nullptr;
	void* pkt = buildPacketFromJson(json, def);
	if (!pkt) return false;
	if (!Hooks::g_LastProcessPacketInstance) {
		Util::log("[PacketSender] TryReceive: no ProcessPacket instance captured yet\n");
		return false;
	}
	Hooks::hkProcessPacket(Hooks::g_LastProcessPacketInstance, (Object*)pkt);
	Util::log("[PacketSender] Injected %s\n", def->name);
	return true;
}

void PacketSender::DumpInteractions() {
	if (!g_LastGameInstance) return;
	void* module = *(void**)((uint8_t*)g_LastGameInstance + 0x150);
	if (!Util::IsValidPtr(module)) return;
	void* arrayPtr = *(void**)((uint8_t*)module + 0x40);
	if (!Util::IsValidPtr(arrayPtr)) return;
	int count = *(int*)((uint8_t*)arrayPtr + 0x08);

	std::string path = (Globals::paths && Util::IsValidPtr(Globals::paths->ClientGameDirectory))
		? Globals::paths->ClientGameDirectory->getString() + "\\interactions_dump.txt"
		: "interactions_dump.txt";

	std::ofstream file(path);
	if (!file.is_open()) return;
	file << "ID | InternalName\n-----------------\n";
	auto* arr = (Array<void*>*)arrayPtr;
	for (int i = 0; i < count; ++i) {
		void* entry = arr->list[i];
		if (!Util::IsValidPtr(entry)) continue;
		HytaleString* nameStr = *(HytaleString**)((uint8_t*)entry + 0x08);
		if (Util::IsValidPtr(nameStr)) file << i << " | " << nameStr->getString() << "\n";
	}
	file.close();
	Util::log("[DumpInteractions] Written to: %s\n", path.c_str());
}

const char* PacketSender::GetPacketName(int index) {
	for (int i = 0; i < PKT_TABLE_SIZE; ++i)
		if (PKT_TABLE[i].index == (PacketIndex)index) return PKT_TABLE[i].name;
	return nullptr;
}

// TODO: implement field reader (inverse of writeField) to populate each JSON field.
std::string PacketSender::PacketToJson(Object* pkt, PacketIndex index) {
	const PacketDef* def = nullptr;
	for (int j = 0; j < PKT_TABLE_SIZE; ++j) {
		if (PKT_TABLE[j].index == index) { def = &PKT_TABLE[j]; break; }
	}
	if (!def || !pkt) return "{}";
	return std::string("{\"name\":\"") + def->name + "\"}";
}

bool PacketSender::CanExecute() { return Util::isFullyInitialized(); }

void PacketSender::Initialize() {
	SubTypeRegistry::Initialize();
}

// Packet Lab Win32 UI

#define ID_BTN_SEND    101
#define ID_BTN_RECEIVE 102
#define ID_EDIT_JSON   103
#define ID_BTN_TRACE   104

static HWND hEdit = NULL;

static std::string getEditText() {
	int len = GetWindowTextLengthA(hEdit);
	if (len <= 0) return "";
	std::string buf(len + 1, '\0');
	GetWindowTextA(hEdit, buf.data(), len + 1);
	buf.resize(len);
	return buf;
}

LRESULT CALLBACK PacketLabWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
		hEdit = CreateWindowExA(
			WS_EX_CLIENTEDGE, "EDIT", "",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
			10, 10, 560, 300, hWnd, (HMENU)ID_EDIT_JSON, NULL, NULL);
		HFONT hFont = CreateFontA(16, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
		CreateWindowA("BUTTON", "Send C2S",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			10, 320, 130, 30, hWnd, (HMENU)ID_BTN_SEND, NULL, NULL);
		CreateWindowA("BUTTON", "Receive S2C",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			150, 320, 130, 30, hWnd, (HMENU)ID_BTN_RECEIVE, NULL, NULL);
		CreateWindowA("BUTTON", "Trace: OFF",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			290, 320, 130, 30, hWnd, (HMENU)ID_BTN_TRACE, NULL, NULL);
		break;
	}
	case WM_COMMAND: {
		if (LOWORD(wParam) == ID_BTN_SEND) {
			std::string json = getEditText();
			if (!json.empty()) PacketSender::TrySend(json);
		}
		if (LOWORD(wParam) == ID_BTN_RECEIVE) {
			std::string json = getEditText();
			if (!json.empty()) PacketSender::TryReceive(json);
		}
		if (LOWORD(wParam) == ID_BTN_TRACE) {
			PacketSender::TracePackets = !PacketSender::TracePackets;
			SetWindowTextA(GetDlgItem(hWnd, ID_BTN_TRACE), PacketSender::TracePackets ? "Trace: ON" : "Trace: OFF");
			Util::log("[PacketLab] Tracing %s\n", PacketSender::TracePackets ? "ON" : "OFF");
		}
		break;
	}
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		return 0;
	}
	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void PacketSender::OpenPacketLabUI() {
	static bool registered = false;
	static HWND hWndLab = NULL;
	if (!registered) {
		WNDCLASSEXA wc = { sizeof(WNDCLASSEXA) };
		wc.lpfnWndProc   = PacketLabWndProc;
		wc.hInstance     = GetModuleHandleA(NULL);
		wc.lpszClassName = "PacketLabClass";
		wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		RegisterClassExA(&wc);
		registered = true;
	}
	if (!hWndLab)
		hWndLab = CreateWindowExA(WS_EX_TOPMOST, "PacketLabClass", "Fish++ Packet Lab",
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
			NULL, NULL, GetModuleHandleA(NULL), NULL);
	ShowWindow(hWndLab, SW_SHOW);
	UpdateWindow(hWndLab);
	SetForegroundWindow(hWndLab);
}
