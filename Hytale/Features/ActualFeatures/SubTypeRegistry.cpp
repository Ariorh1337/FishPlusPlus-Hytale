/*
 * Copyright (c) FishPlusPlus.
 */
#include "SubTypeRegistry.h"
#include "core.h"
#include "Util/Util.h"
#include "Util/SigManager.h"

#include <unordered_map>
#include <string>
#include <string_view>

using namespace PacketTable;

// ─────────────────────────────────────────────────────────────────────────────
// Cache: struct name (from ptr_type) → MethodTable*
// ─────────────────────────────────────────────────────────────────────────────

static std::unordered_map<std::string, void*> g_cache;

void SubTypeRegistry::Initialize() {
    auto add = [](const char* name, uint64_t offset) {
        if (!g_cache.count(name))
            g_cache[name] = (void*)(gameBase + offset);
    };

    add("MovementSettings", 0x1B16358);
    add("FormattedMessage", 0x1B15280);
    add("FormattedMessageImage", 0x1E7FB30);
    add("Array<int>", 0x22218B0);
    add("ForkedChainId", 0x1B19968);
    add("Position", 0x1B14668);
    add("Direction", 0x1B14950);
    add("Vector3d", 0x1B148A0);
    add("DamageCause", 0x1B11B08);
    add("InventorySection", 0x1B16620);
    add("ModelTransform", 0x1B14A30);
    add("String", 0x1BC6EA0);
    add("SavedMovementStates", 0x1B16408);
    add("Array<HudComponent>", 0x21FD2A0);
    add("Array<Byte>", 0x2220FF8);
    add("ItemWithAllMetadata", 0x1B164D0);
}

// Learn a MethodTable under a known name (taken directly from fd.ptr_type).
// No ObjectToString needed — we already know the type name from the table.
static void tryLearn(const char* type_name, void* obj_ptr) {
    if (!obj_ptr || !Util::IsValidPtr(obj_ptr)) return;
    if (g_cache.count(type_name)) return;  // already cached

    void* mt = *(void**)obj_ptr;
    if (!Util::IsValidPtr(mt)) return;

    g_cache[type_name] = mt;
    uint64_t offset = (uint64_t)mt - gameBase;
    Util::log("[SubTypeReg] Learned '%s'  offset=0x%llX  (add to static table)\n",
              type_name, offset);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void SubTypeRegistry::ScanPacket(Object* packet, PacketIndex index) {
    const PacketDef* def = nullptr;
    for (int i = 0; i < PKT_TABLE_SIZE; ++i) {
        if (PKT_TABLE[i].index == index) { def = &PKT_TABLE[i]; break; }
    }
    if (!def) return;

    auto* base = (uint8_t*)packet;

    for (int i = 0; i < def->field_count; ++i) {
        const FieldDesc& fd = def->fields[i];
        if (fd.type != FType::Ptr || !fd.ptr_type) continue;

        void* child = *(void**)(base + fd.offset);
        if (!child) continue;

        tryLearn(fd.ptr_type, child);

        // One level deeper
        const SubTypeDef* childDef = nullptr;
        for (int j = 0; j < SUB_TYPE_TABLE_SIZE; ++j) {
            if (std::string_view(SUB_TYPE_TABLE[j].name) == fd.ptr_type) {
                childDef = &SUB_TYPE_TABLE[j]; break;
            }
        }
        if (!childDef) continue;

        auto* childBase = (uint8_t*)child;
        for (int j = 0; j < childDef->field_count; ++j) {
            const FieldDesc& cfd = childDef->fields[j];
            if (cfd.type != FType::Ptr || !cfd.ptr_type) continue;
            void* grandChild = *(void**)(childBase + cfd.offset);
            if (grandChild) tryLearn(cfd.ptr_type, grandChild);
        }
    }
}

void* SubTypeRegistry::GetMethodTable(const char* type_name) {
    auto it = g_cache.find(type_name);
    return it != g_cache.end() ? it->second : nullptr;
}

void* SubTypeRegistry::Alloc(const char* type_name) {
    void* mt = GetMethodTable(type_name);
    if (!mt) return nullptr;

    using RhpNewFastFn = void*(*)(void*);
    static RhpNewFastFn RhpNewFast = reinterpret_cast<RhpNewFastFn>(SM::RhpNewFastAddress);
    return RhpNewFast(mt);
}

int SubTypeRegistry::CachedCount() {
    return (int)g_cache.size();
}
