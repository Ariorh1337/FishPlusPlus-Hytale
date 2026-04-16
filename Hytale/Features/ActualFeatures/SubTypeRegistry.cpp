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

static std::unordered_map<std::string, void*> g_cache;

// Statically seeded MethodTable offsets, all relative to gameBase.
// Verified against Hytale 0.5.x (HytaleClient.exe).
// Types marked UNVERIFIED were inferred from static analysis — confirm via ScanPacket log.
// Types in comments (???) haven't been located yet; they will be learned at runtime.
void SubTypeRegistry::Initialize() {
	auto add = [](const char* name, uint64_t offset) {
		if (!g_cache.count(name))
			g_cache[name] = (void*)(gameBase + offset);
	};

	// Primitives / math
	add("Position",            0x1B14668); // f64 x/y/z — ClientMovement, SyncInteractionChains
	add("Direction",           0x1B14950); // f64 yaw/pitch — ClientMovement
	add("Vector3d",            0x1B148A0); // f64 x/y/z world-space variant
	add("BlockPosition",       0x1B14610); // i32 x/y/z — SyncInteractionChains, ClientPlaceBlock
	add("ModelTransform",      0x1B14A30); // UNVERIFIED

	// Simple data
	add("String",              0x1BC6EA0); // HytaleString — emote_id, animation_id, ...
	add("InstantData",         0x1B19DE8); // Ping / Pong timestamp wrapper
	add("ForkedChainId",       0x1B19968); // CancelInteractionChain, PlayInteractionFor
	add("SavedMovementStates", 0x1B16408); // bool flying — ClientMovement
	add("DamageCause",         0x1B11B08); // UNVERIFIED
	add("ExtraResources",      0x1B165B0); // UNVERIFIED

	// Inventory / items
	add("ItemWithAllMetadata", 0x1B164D0); // UpdatePlayerInventory
	add("InventorySection",    0x1B16620); // UNVERIFIED
	add("MovementSettings",    0x1B16358); // UpdateMovementSettings_S2C.settings

	// Interaction chain types
	add("SyncInteractionChain",   0x1B1EB18); // SyncInteractionChains_BI.updates[]
	add("InteractionChainData",   0x1B199D8); // SyncInteractionChain.data
	add("InteractionSyncData",    0x1B19AB8); // SyncInteractionChain.interaction_data[]

	// Entity updates
	add("EntityUpdate",           0x1B17410); // EntityUpdates_S2C.updates[]
	add("ComponentUpdate",        0x1B16CF0); // EntityUpdate.component_updates[]

	// Formatted messages
	add("FormattedMessage",       0x1B15280); // ServerDisconnect, ServerMessage
	add("FormattedMessageImage",  0x1E7FB30); // UNVERIFIED

	// Map / world
	add("MapMarker",              0x1B1B070);
	add("MapMarkerComponent",     0x1B1B270);
	add("MapChunk",               0x1B1AF90);
	add("MapImage",               0x1B1AF20);
	add("SetBlockCmd",            0x1B1A2D8);
	add("Transform",              0x1B149C0);

	// Items / inventory
	add("Asset",                  0x1B19D20);
	add("ItemCategory",           0x1B12650);
	add("SubCategoryDefinition",  0x1B125E0);

	// Players
	add("ServerPlayerListPlayer", 0x1B1DD60);
	add("ServerPlayerListUpdate", 0x1B1DDB8);

	// Array MethodTables (needed by RhpNewArray, not RhpNewFast)
	add("Array<int>",                       0x22218B0);
	add("Array<Byte>",                      0x2220FF8);
	add("Array<Guuid>",                     0x22215F8);
	add("Array<HudComponent>",              0x21FD2A0); // UNVERIFIED
	add("Array<String*>",                   0x1E7C4C0);
	add("Array<FormattedMessage*>",         0x21FB048);
	add("Array<ItemCategory*>",             0x21F8AC8);
	add("Array<SubCategoryDefinition*>",    0x21F8A10);
	add("Array<ItemQuantity*>",             0x21FB8B8);
	add("Array<MapMarker*>",                0x21FCE90);
	add("Array<MapMarkerComponent*>",       0x21FCF48);
	add("Array<MapChunk*>",                 0x21FCD20);
	add("Array<SetBlockCmd*>",              0x21FCBB0);
	add("Array<ModelParticle*>",            0x21F7E00);
	add("Array<EntityUpdate*>",             0x21FBD08);
	add("Array<ComponentUpdate*>",          0x21FBC50);
	add("Array<ComponentUpdateType>",       0x21FB4E8);
	add("Array<CustomUICommand*>",          0x21FD3B8);
	add("Array<CustomUIEventBinding*>",     0x21FD470);
	add("Array<ServerPlayerListPlayer*>",   0x21FD698);
	add("Array<ServerPlayerListUpdate*>",   0x21FD750);
	add("Array<SyncInteractionChain*>",     0x21FD808);
	add("Array<InteractionSyncData*>",      0x21FC7D0);

	// Not yet found — will be learned at runtime via ScanPacket:
	//   add("MovementStates",    ???);  // MountMovement_C2S.movement_states
	//   add("VelocityConfig",    ???);  // ChangeVelocity_S2C.config
	//   add("Vector3f",          ???);  // SelectableEntity, ServerCameraSettings, ...
	//   add("Vector2f",          ???);  // ServerCameraSettings.look_multiplier
	//   add("SelectedHitEntity", ???);  // MouseInteraction_C2S.selected_entity
	//   add("HudComponent",      ???);  // CustomHud_S2C.components[]
}

void* SubTypeRegistry::Alloc(const char* type_name) {
	void* mt = GetMethodTable(type_name);
	if (!mt) return nullptr;
	using RhpNewFastFn = void*(*)(void*);
	static RhpNewFastFn RhpNewFast = reinterpret_cast<RhpNewFastFn>(SM::RhpNewFastAddress);
	return RhpNewFast(mt);
}

void* SubTypeRegistry::AllocArray(const char* type_name, int count) {
	void* mt = GetMethodTable(type_name);
	if (!mt || !SM::RhpNewArray_GenericAddress) return nullptr;
	using RhpNewArrayFn = void*(*)(void*, int);
	return reinterpret_cast<RhpNewArrayFn>(SM::RhpNewArray_GenericAddress)(mt, count);
}

void* SubTypeRegistry::GetMethodTable(const char* type_name) {
	auto it = g_cache.find(type_name);
	return it != g_cache.end() ? it->second : nullptr;
}

int SubTypeRegistry::CachedCount() {
	return (int)g_cache.size();
}

// RUNTIME LEARNING
// Scans pointer fields of received packets to discover unknown MethodTables.
// New finds are logged: [SubTypeReg] Learned 'TypeName'  offset=0xXXXX  (add to static table)
// Copy the offset into Initialize() above, then delete tryLearn/ScanObject/ScanPacket
// and the ScanPacket call in ProcessPacket.cpp.

static void tryLearn(const char* type_name, void* obj_ptr) {
	if (!obj_ptr || !Util::IsValidPtr(obj_ptr)) return;
	if (g_cache.count(type_name)) return;

	void* mt = *(void**)obj_ptr;
	if (!Util::IsValidPtr(mt)) return;

	g_cache[type_name] = mt;
	Util::log("[SubTypeReg] Learned '%s'  offset=0x%llX  (add to static table)\n",
		type_name, (uint64_t)mt - gameBase);
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
			def = &SUB_TYPE_TABLE[j];
			break;
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
