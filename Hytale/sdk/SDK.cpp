/*
 * Copyright (c) FishPlusPlus.
 */
#include "SDK.h"

#include "Events/EventRegister.h"
#include "Features/FeatureHandler.h"
#include "Features/ActualFeatures/BlockESP.h"
#include "Packets/ClientBlockPlace.h"
#include "Hooks/Hooks.h"
#include "BaseDataTypes/MethodTable.h"
#include "Packets/SyncInteractionChains.h"
#include <fstream>
#include <shlobj.h>
#include <algorithm>

 // Print the name and address of an object, and scan its fields for pointers
#define DBGScan(object) if (object)	ScanObject((void*)object);																		

// Print the name and address of an object, if it exists
#define DBGName(object) if (object) {																				\
	HytaleString* object##Name = SafeObjectToString((void*)object);													\
	Util::log("%s: %s @ 0x%llX\n", #object, object##Name ? object##Name->getString().c_str() : "nullptr", object);	\
}

std::vector<EntityData> getEntities(Entity* localPlayer) {
	GameInstance* gameInstance = Util::getGameInstance();
	ValidPtrEmpty(gameInstance);

	EntityStoreModule* entityStoreModule = gameInstance->EntityStoreModule;
	ValidPtrEmpty(entityStoreModule);

	int entityCount = entityStoreModule->entityCount;
	Array<Entity*>* entityArray = entityStoreModule->entityArray;

	std::vector<EntityData> entities;
	for (int i = 0; i < entityCount; i++) {
		Entity* entity = entityArray->get(i);
		ValidPtrLoop(entity);

		EntityAssetStruct* assetStruct = entity->AssetNameStruct;

		EntityData data;
		data.entityPtr = entity;
		data.player = entity->IsAPlayer();
		if (data.player)
			data.name = entity->Name->getString();
		else if (Util::IsValidPtr(assetStruct)) {
			HytaleString* entityString = assetStruct->entityString;
			data.name = entityString->getString();
		} else
			data.name = "Unknown";

		data.entityType = entity->entityType;
		data.networkID = entity->networkId;
		data.position = entity->Position;
		data.isLocalPlayer = (entity == localPlayer);

		entities.push_back(data);
	}
	return entities;
}

void UpdateInputStates(bool skipResetKeys) {
	using m_UpdateInputStates = uint64_t(*)(AppInGame* AppGame, bool skipResetKeys);
	static m_UpdateInputStates UpdateInputStates_method{ };
	if (!UpdateInputStates_method)
		UpdateInputStates_method = reinterpret_cast<m_UpdateInputStates>(SM::UpdateInputStatesAddress);
	App* app = Util::app;
	if (!app || !app->appInGame) {
		Util::log("Invalid app or appInGame pointer\n");
		return;
	}
	UpdateInputStates_method(app->appInGame, skipResetKeys);
}

void setCursorHidden(bool hidden) {
	using m_SetCursorHidden = void(*)(Window* window, bool hidden);
	static m_SetCursorHidden SetCursorHidden_method{ };
	if (!SetCursorHidden_method)
		SetCursorHidden_method = reinterpret_cast<m_SetCursorHidden>(SM::SetCursorHiddenAddress);
	App* app = Util::app;
	if (!app || !app->Engine || !app->Engine->Window) {
		Util::log("Invalid app or window pointer\n");
		return;
	}
	SetCursorHidden_method(app->Engine->Window, hidden);
}

void* SafeReadPointer(void** ptrLocation) {
	__try {
		return *ptrLocation;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return (void*) -1;
	}
}

bool IsValidObject(void* ptr) {
	if (!ptr)
		return false;

	uintptr_t ptrValue = (uintptr_t) ptr;

	if (ptrValue % 8 != 0)
		return false;

	if (ptrValue < 0x10000 || ptrValue > 0x7FFFFFFFFFFF)
		return false;

	__try {
		MethodTable* mt = (MethodTable*)SafeReadPointer((void**) ptr);
		if (!mt)
			return false;

		uintptr_t mtValue = (uintptr_t) mt;
		if (mtValue < 0x10000 || mtValue > 0x7FFFFFFFFFFF)
			return false;

		volatile uint32_t flags = mt->m_uFlags;
		volatile uint16_t numSlots = mt->m_usNumVtableSlots;
		volatile uint32_t baseSize = mt->m_uBaseSize;

		if (numSlots > 10000)
			return false;
		if (baseSize == 0 || baseSize > 0x100000)
			return false;

		return true;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

HytaleString* SafeObjectToString(void* ptr) {
	if(!IsValidObject(ptr))
		return nullptr;
	__try {
		return Util::ObjectToString(ptr);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

void ScanObject(void* object) {
	HytaleString* scan_obj_name = SafeObjectToString(object);
	Util::log("=== Scanning %s at 0x%llX ===\n", scan_obj_name->getString().c_str(), (uint64_t) object);
	for (int offset = 0x8; offset < 0x300; offset += 0x8) {
		void** ptrLocation = (void**) ((uint64_t) object + offset);
		void* ptr = SafeReadPointer(ptrLocation);

		if (ptr == (void*) -1) {
			Util::log("Offset 0x%X: Access violation reading pointer\n", offset);
		} else if (ptr) {
			HytaleString* obj_name = SafeObjectToString(ptr);
			if (obj_name) {
				std::string nameStr = obj_name->getString();
				Util::log("Offset 0x%X (Ptr: 0x%llX): %s\n",
					offset,
					(uint64_t) ptr,
					nameStr.c_str());
			} else {
				Util::log("Offset 0x%X (Ptr: 0x%llX): Exception calling ObjectToString\n",
					offset,
					(uint64_t) ptr);
			}
		} else {
			Util::log("Offset 0x%X: nullptr\n", offset);
		}
	}
	Util::log("=== Finished scan ===\n");
}

void SDK::Reset() {
	if (!filterInitialized)
		return;
	filterInitialized = false;

	SDK::filteredBlockMutex.lock();
	filteredBlocks.clear();
	SDK::filteredBlockMutex.unlock();
	for (RenderBlockInfo& filter : ImportantBlocks)
		filter.BlockID.clear();
	allTargetBlockIds.clear();
	lookupTable.clear();
}

void SDK::Main() {
	if (Menu::isMenuOpen() && Util::app->Engine->Window->IsCursorHidden) {
		setCursorHidden(false);
	}

	if (!Menu::isMenuOpen() && Menu::m_justClosed) {
		UpdateInputStates(true);
		Menu::m_justClosed = false;
	}

	if (!Util::app || !Util::app->appInGame || !Util::app->appInGame->gameInstance || Util::app->Stage != AppStage::InGame) {
		Reset();
		return;
	}

	if (!FeatureHandler::FeaturesLoaded()) {
		Reset();
		return;
	}

	global_mutex.lock();
	entities = getEntities(Util::getLocalPlayer());
	global_mutex.unlock();

	static bool firstScan = false;
	if (firstScan || (GetAsyncKeyState(VK_F5) & 1)) {
		Vector3 playerPos = Util::getLocalPlayer()->Position.toFloor().add(0,-1,0);
		Util::log("Player position: (%.1f, %.1f, %.1f)\n", playerPos.x, playerPos.y, playerPos.z);
		Util::log("Block at player position: %s\n", Util::getGameInstance()->MapModule->GetBlockType(playerPos)->Name->getString().c_str());

		SyncInteractionChainsPacket* packet = CreatePacket<SyncInteractionChainsPacket*>(SyncInteractionChains_BI);

		BlockPosition* blockPos = API::RHPNewFast<BlockPosition*>(SM::BlockPosition_MTAddress);
		blockPos->x = (int) playerPos.x;
		blockPos->y = (int) playerPos.y;
		blockPos->z = (int) playerPos.z;

		InteractionChainData* data = API::RHPNewFast<InteractionChainData*>(SM::InteractionChainData_MTAddress);
		data->block_position = blockPos;
		data->entity_id = -1;
		data->target_slot = INT_MIN;

		auto CreateInteractionSyncData = [&]() -> InteractionSyncData* {
			auto* sd = API::RHPNewFast<InteractionSyncData*>(SM::InteractionSyncData_MTAddress);
			sd->entered_root_interaction = INT_MIN;
			sd->placed_block_id          = INT_MIN;
			sd->charge_value             = -1.0f;
			sd->chaining_index           = -1;
			sd->flag_index               = -1;
			return sd;
		};

		InteractionSyncData* syncData0 = CreateInteractionSyncData();
		syncData0->operation_counter = 0;
		syncData0->root_interaction = 2829;
		syncData0->state = kInteractionStateFinished;

		InteractionSyncData* syncData1 = CreateInteractionSyncData();
		syncData1->operation_counter = 1;
		syncData1->root_interaction = 2829;
		syncData1->block_position = blockPos;
		syncData1->block_face = BlockFace::kBlockFaceUp;
		syncData1->state = kInteractionStateFinished;

		InteractionSyncData* syncData2 = CreateInteractionSyncData();
		syncData2->operation_counter = 0;
		syncData2->root_interaction = 2628;
		syncData2->block_position = blockPos;
		syncData2->block_face = BlockFace::kBlockFaceUp;
		syncData2->state = kInteractionStateFinished;

		InteractionSyncData* syncData3 = CreateInteractionSyncData();
		syncData3->operation_counter = 2;
		syncData3->root_interaction = 2829;
		syncData3->state = kInteractionStateFinished;

		Array<InteractionSyncData*>* updatesArray = API::RHPNewArray<Array<InteractionSyncData*>*>(SM::Array_InteractionSyncData_MTAddress, 4);
		updatesArray->list[0] = syncData0;
		updatesArray->list[1] = syncData1;
		updatesArray->list[2] = syncData2;
		updatesArray->list[3] = syncData3;

		SyncInteractionChain* chain = API::RHPNewFast<SyncInteractionChain*>(SM::SyncInteractionChain_MTAddress);
		chain->interaction_type = InteractionType::kInteractionTypeUse;
		chain->active_hotbar_slot = Util::getGameInstance()->InventoryModule->HotbarActiveSlot;
		chain->active_utility_slot = -1;
		chain->active_tools_slot = -1;
		chain->override_root_interaction = INT_MIN;
		chain->equip_slot = Util::getGameInstance()->InventoryModule->HotbarActiveSlot;
		chain->chain_id = ChainID;
		chain->operation_base_index = 0;
		chain->initial = true;
		chain->data = data;
		chain->interaction_data = updatesArray;

		Array<SyncInteractionChain*>* updates = API::RHPNewArray<Array<SyncInteractionChain*>*>(SM::Array_SyncInteractionChain_MTAddress, 1);
		updates->list[0] = chain;

		packet->updates = updates;

		Packets::SendPacketImmediate(packet);
		Util::log("Sent SyncInteractionChainsPacket with test data\n");

		firstScan = false;
	}

	if (BlockESP* blockESP = (BlockESP*) FeatureHandler::GetFeatureFromName("BlockESP"); blockESP != nullptr && blockESP->refreshList != nullptr && (blockESP->refreshList->GetValue() || !filterInitialized)) {
		Reset();
		MapModule* mapModule = (MapModule*) Util::app->appInGame->gameInstance->MapModule;
		Array<ClientBlockType*>* ClientBlockTypes = mapModule->ClientBlockTypes;

		for (int i = 0; i < ClientBlockTypes->count; i++) {
			ClientBlockType* blockType = ClientBlockTypes->get(i);
			if (blockType) {
				std::string blockName = blockType->Name->getString();
				for (RenderBlockInfo& filter : ImportantBlocks) {
					if (blockName.find(filter.DisplayName) != std::string::npos && blockESP->IsBlockImportant(filter.settingIndex)) {
						filter.BlockID.push_back(blockType->Id);
						allTargetBlockIds.push_back(blockType->Id);
					}
				}
			}
		}
		filterInitialized = true;
		blockESP->refreshList->SetValue(false);
	}
}