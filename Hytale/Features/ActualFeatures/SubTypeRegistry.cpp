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
    add("BlockPosition", 0x1B14610);
    add("Array<ItemCategory*>", 0x21F8AC8);
    add("Array<String*>", 0x1E7C4C0);
    add("Array<ServerPlayerListPlayer*>", 0x21FD698);
    add("Array<ServerPlayerListUpdate*>", 0x21FD750);
    add("Array<EntityUpdate*>", 0x21FBD08);
    add("Array<CustomUICommand*>", 0x21FD3B8);
    add("Array<CustomUIEventBinding*>", 0x21FD470);
    add("InstantData", 0x1B19DE8);
    add("Array<MapMarker*>", 0x21FCE90);
    add("Array<MapChunk*>", 0x21FCD20);
    add("Array<SyncInteractionChain*>", 0x21FD808);
    add("Array<SetBlockCmd*>", 0x21FCBB0);
    add("ExtraResources", 0x1B165B0);
    add("Array<ItemQuantity*>", 0x21FB8B8);
    add("Array<ModelParticle*>", 0x21F7E00);
    add("EntityUpdate", 0x1B17410);
    add("Array<ComponentUpdate*>", 0x21FBC50);
    add("ComponentUpdate", 0x1B16CF0);
    add("SyncInteractionChain", 0x1B1EB18);
    add("InteractionChainData", 0x1B199D8);
    add("Array<InteractionSyncData*>", 0x21FC7D0);
    add("InteractionSyncData", 0x1B19AB8);
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

static void ScanObject(const char* type_name, void* obj, int depth = 0) {
    if (!obj || !Util::IsValidPtr(obj) || depth > 5) return;
    tryLearn(type_name, obj);

    std::string_view p_type = type_name;
    if (p_type.starts_with("Array<")) {
        std::string inner_type = std::string(p_type.substr(6, p_type.size() - 7));
        bool is_ptr = inner_type.ends_with("*");
        if (is_ptr) {
            inner_type.pop_back();
            int count = *(int*)((uint8_t*)obj + 0x08);
            if (count > 0 && count < 1000) {
                uint64_t* list = (uint64_t*)((uint8_t*)obj + 0x10);
                for (int k = 0; k < count; ++k) {
                    if (list[k]) ScanObject(inner_type.c_str(), (void*)list[k], depth + 1);
                }
            }
        }
        return;
    }

    const SubTypeDef* def = nullptr;
    for (int j = 0; j < SUB_TYPE_TABLE_SIZE; ++j) {
        if (std::string_view(SUB_TYPE_TABLE[j].name) == type_name) {
            def = &SUB_TYPE_TABLE[j]; break;
        }
    }
    if (!def) return;

    auto* base = (uint8_t*)obj;
    for (int j = 0; j < def->field_count; ++j) {
        const FieldDesc& fd = def->fields[j];
        if (fd.type != FType::Ptr || !fd.ptr_type) continue;
        void* child = *(void**)(base + fd.offset);
        if (child) ScanObject(fd.ptr_type, child, depth + 1);
    }
}

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
        if (child) ScanObject(fd.ptr_type, child, 0);
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
