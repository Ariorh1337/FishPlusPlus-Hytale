/*
 * Copyright (c) FishPlusPlus.
 */
#include "SDK.h"

#include "Events/EventRegister.h"
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
		ValidPtrLoop(assetStruct);

		HytaleString* entityString = assetStruct->entityString;
		ValidPtrLoop(assetStruct);

		EntityData data;
		data.entityPtr = entity;
		data.player = entity->IsAPlayer();
		if (data.player)
			data.name = entity->Name->getString();
		else
			data.name = entityString->getString();
		data.entityType = entity->entityType;
		data.networkID = entity->networkId;
		data.position = entity->Position;
		data.isLocalPlayer = (entity == localPlayer);

		//printf("Entity %d: %s - 0x%llX\n", i, name.c_str(), entity);

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

class MethodTable {
public: // private in official codebase
	struct RelatedTypeUnion {
		union {
			// Kinds.CanonicalEEType
			MethodTable* m_pBaseType;

			// Kinds.ParameterizedEEType
			MethodTable* m_pRelatedParameterType;
		};
	};

	// native code counterpart for _uFlags
	union {
		uint32_t              m_uFlags;
		// lower uint16 of m_uFlags is ComponentSize, when HasComponentSize == true
		// also accessed in asm allocation helpers
		uint16_t              m_usComponentSize;
	};
	uint32_t              m_uBaseSize;
	RelatedTypeUnion      m_RelatedType;
	uint16_t              m_usNumVtableSlots;
	uint16_t              m_usNumInterfaces;
	uint32_t              m_uHashCode;

	void* m_VTable[];  // make this explicit so the binder gets the right alignment

	// after the m_usNumVtableSlots vtable slots, we have m_usNumInterfaces slots of
	// MethodTable*, and after that a couple of additional pointers based on whether the type is
	// finalizable (the address of the finalizer code) or has optional fields (pointer to the compacted
	// fields).

	enum Flags {
		// There are four kinds of EETypes, the three of them regular types that use the full MethodTable encoding
		// plus a fourth kind used as a grab bag of unusual edge cases which are encoded in a smaller,
		// simplified version of MethodTable. See LimitedEEType definition below.
		EETypeKindMask = 0x00030000,

		// GC depends on this bit, this bit must be zero
		CollectibleFlag = 0x00200000,

		HasDispatchMapFlag = 0x00040000,

		IsDynamicTypeFlag = 0x00080000,

		// GC depends on this bit, this type requires finalization
		HasFinalizerFlag = 0x00100000,

		HasSealedVTableEntriesFlag = 0x00400000,

		// GC depends on this bit, this type contain gc pointers
		HasPointersFlag = 0x01000000,

		// This type is generic and one or more of it's type parameters is co- or contra-variant. This only
		// applies to interface and delegate types.
		GenericVarianceFlag = 0x00800000,

		// This type is generic.
		IsGenericFlag = 0x02000000,

		// We are storing a EETypeElementType in the upper bits for unboxing enums
		ElementTypeMask = 0x7C000000,
		ElementTypeShift = 26,

		// The m_usComponentSize is a number (not holding ExtendedFlags).
		HasComponentSizeFlag = 0x80000000,
	};

	enum ExtendedFlags {
		HasEagerFinalizerFlag = 0x0001,
		// GC depends on this bit, this type has a critical finalizer
		HasCriticalFinalizerFlag = 0x0002,
		IsTrackedReferenceWithFinalizerFlag = 0x0004,

		// This MethodTable is for a Byref-like class (TypedReference, Span<T>, ...)
		IsByRefLikeFlag = 0x0010,

		// This type requires 8-byte alignment for its fields on certain platforms (ARM32, WASM)
		RequiresAlign8Flag = 0x1000
	};

	enum FunctionPointerFlags {
		IsUnmanaged = 0x80000000,
		FunctionPointerFlagsMask = IsUnmanaged
	};
};

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

// Block scanning state
static int playerCenterChunkX = INT_MIN;
static int playerCenterChunkZ = INT_MIN;
static int scanIndex = 0;
static int scanChunkY = 0;
static int scanSlice = 0;
static int framesSinceLastScan = 0;

static const int SCAN_RADIUS = 3;
static const int SEARCH_INTERVAL_FRAMES = 1;
static const int MAX_CHUNKS_Y = 8;
static const int SLICES_PER_CHUNK = 16;

static std::vector<std::pair<int, int>> spiralOrder;
static std::vector<int> allTargetBlockIds;

void InitializeSpiralOrder() {
	if (!spiralOrder.empty()) return;

	spiralOrder.push_back({0, 0});

	int layer = 1;
	while (layer <= SCAN_RADIUS) {
		for (int z = -layer + 1; z <= layer; z++)
			spiralOrder.push_back({layer, z});
		for (int x = layer - 1; x >= -layer; x--)
			spiralOrder.push_back({x, layer});
		for (int z = layer - 1; z >= -layer; z--)
			spiralOrder.push_back({-layer, z});
		for (int x = -layer + 1; x < layer; x++)
			spiralOrder.push_back({x, -layer});
		layer++;
	}
}

void SDK::ScanForBlocks() {
	if (Util::app->Stage != AppStage::InGame)
		return;

	MapModule* mapModule = Util::getGameInstance()->MapModule;
	if (!mapModule)
		return;

	if (allTargetBlockIds.empty())
		allTargetBlockIds = mapModule->getAllImportantBlockIDs();
	if (allTargetBlockIds.empty())
		return;

	Vector3 playerPos = Util::getLocalPlayer()->Position;
	int currentPlayerChunkX = (int)floor(playerPos.x) >> 5;
	int currentPlayerChunkZ = (int)floor(playerPos.z) >> 5;

	InitializeSpiralOrder();

	// Player moved to new chunk - cleanup out-of-bounds blocks
	if (currentPlayerChunkX != playerCenterChunkX || currentPlayerChunkZ != playerCenterChunkZ) {
		playerCenterChunkX = currentPlayerChunkX;
		playerCenterChunkZ = currentPlayerChunkZ;

		int minChunkX = playerCenterChunkX - SCAN_RADIUS;
		int maxChunkX = playerCenterChunkX + SCAN_RADIUS;
		int minChunkZ = playerCenterChunkZ - SCAN_RADIUS;
		int maxChunkZ = playerCenterChunkZ + SCAN_RADIUS;

		SDK::filteredBlocks.erase(
			std::remove_if(SDK::filteredBlocks.begin(), SDK::filteredBlocks.end(),
				[minChunkX, maxChunkX, minChunkZ, maxChunkZ](const FilteredBlockResult& block) {
					int blockChunkX = (int)floor(block.position.x) >> 5;
					int blockChunkZ = (int)floor(block.position.z) >> 5;
					return blockChunkX < minChunkX || blockChunkX > maxChunkX ||
						   blockChunkZ < minChunkZ || blockChunkZ > maxChunkZ;
				}),
			SDK::filteredBlocks.end()
		);

		scanIndex = 0;
		scanChunkY = 0;
		scanSlice = 0;
	}

	framesSinceLastScan++;
	if (framesSinceLastScan < SEARCH_INTERVAL_FRAMES)
		return;

	framesSinceLastScan = 0;

	if (scanIndex >= spiralOrder.size())
		scanIndex = 0;

	auto [offsetX, offsetZ] = spiralOrder[scanIndex];
	int scanChunkX = playerCenterChunkX + offsetX;
	int scanChunkZ = playerCenterChunkZ + offsetZ;

	// Calculate slice bounds in local chunk coordinates (0-31)
	int sliceMinX = scanSlice * 8;        // 0, 8, 16, 24
	int sliceMaxX = sliceMinX + 7;        // 7, 15, 23, 31

	// Remove old results from this chunk slice before scanning
	int minY = scanChunkY * 32;
	int maxY = (scanChunkY + 1) * 32 - 1;
	int minX = scanChunkX * 32 + sliceMinX;
	int maxX = scanChunkX * 32 + sliceMaxX;
	int minZ = scanChunkZ * 32;
	int maxZ = minZ + 31;

	SDK::filteredBlocks.erase(
		std::remove_if(SDK::filteredBlocks.begin(), SDK::filteredBlocks.end(),
			[minX, maxX, minY, maxY, minZ, maxZ](const FilteredBlockResult& block) {
				return block.position.x >= minX && block.position.x <= maxX &&
					   block.position.y >= minY && block.position.y <= maxY &&
					   block.position.z >= minZ && block.position.z <= maxZ;
			}),
		SDK::filteredBlocks.end()
	);

	// Scan only the specific slice (8 blocks wide in X, full Y and Z for this chunk)
	std::vector<FishBlockData> foundBlocks = mapModule->FindBlocksInSingleChunk(
		scanChunkX, scanChunkY, scanChunkZ, allTargetBlockIds,
		sliceMinX, sliceMaxX,  // X bounds (local chunk coords)
		0, 31,                  // Y bounds (full chunk height)
		0, 31                   // Z bounds (full chunk depth)
	);

	// Add found blocks directly (no filtering needed - already scanned only the slice)
	for (const auto& blockData : foundBlocks) {
		for (const auto& importantBlock : ImportantBlocks) {
			auto it = std::find(importantBlock.BlockID.begin(), importantBlock.BlockID.end(), blockData.blockId);
			if (it != importantBlock.BlockID.end()) {
				FilteredBlockResult result;
				result.position = Vector3((float)blockData.x, (float)blockData.y, (float)blockData.z);
				result.blockId = blockData.blockId;
				result.displayName = importantBlock.DisplayName;
				result.color = importantBlock.BlockColor;
				SDK::filteredBlocks.push_back(result);
				break;
			}
		}
	}

	// Advance scan position
	scanSlice++;
	if (scanSlice >= SLICES_PER_CHUNK) {
		scanSlice = 0;
		scanChunkY++;
		if (scanChunkY >= MAX_CHUNKS_Y) {
			scanChunkY = 0;
			scanIndex++;
			if (scanIndex >= spiralOrder.size())
				scanIndex = 0;
		}
	}
}

void SDK::Reset() {
	filterInitialized = false;

	SDK::filteredBlockMutex.lock();
	filteredBlocks.clear();
	SDK::filteredBlockMutex.unlock();
	for (RenderBlockInfo& filter : ImportantBlocks)
		filter.BlockID.clear();

}

void SDK::Main() {
	if (Menu::isMenuOpen() && Util::app->Engine->Window->IsCursorHidden) {
		setCursorHidden(false);
	}

	if (!Menu::isMenuOpen() && Menu::m_justClosed) {
		UpdateInputStates(true);
		Menu::m_justClosed = false;
	}

	if (Util::app->Stage != AppStage::InGame) {
		Reset();
		return;
	}


	global_mutex.lock();
	entities = getEntities(Util::getLocalPlayer());
	global_mutex.unlock();

	// Scan for important blocks
	//ScanForBlocks();

	if (!filterInitialized && Util::app && Util::app->appInGame && Util::app->appInGame->gameInstance && Util::app->Stage == AppStage::InGame){
		MapModule* mapModule = (MapModule*) Util::app->appInGame->gameInstance->MapModule;
		Array<ClientBlockType*>* ClientBlockTypes = mapModule->ClientBlockTypes;

		for (RenderBlockInfo& filter : ImportantBlocks) {
			filter.BlockID.clear();
		}

		for (int i = 0; i < ClientBlockTypes->count; i++) {
			ClientBlockType* blockType = ClientBlockTypes->get(i);
			if (blockType->Id == 5) {
				Util::log("Block id 5: %s\n", blockType->Name->getString().c_str());
			}
			if (blockType) {
				std::string blockName = blockType->Name->getString();
				for (RenderBlockInfo& filter : ImportantBlocks) {
					if (blockName.find(filter.DisplayName) != std::string::npos)
						filter.BlockID.push_back(blockType->Id);
				}
			}
		}

		Util::log("[SDK] ImportantBlocks population complete. Summary:\n");
		for (const RenderBlockInfo& filter : ImportantBlocks)
			Util::log("[SDK]   - %s: %d block IDs found\n", filter.DisplayName, filter.BlockID.size());
		filterInitialized = true;

		ICameraController* cameraController = Util::getGameInstance()->CameraModule->Controller;
		Util::log("CameraController: 0x%llX\n", (uintptr_t) cameraController);
		Util::log("MapModule: 0x%llX\n", (uintptr_t) mapModule); //99662D50

		BlockPlacementPreview* module = Util::getGameInstance()->InteractionModule->BlockPreview;
		DBGScan(module);

	}

	
	/*
	static int test = 0;
	if (test < 1 && Util::app && Util::app->appInGame && Util::app->appInGame->gameInstance && Util::app->Stage == AppStage::InGame) {

		//DBGScan(Util::getGameInstance()->MapModule->MapGeometryBuilder);
		Util::log("MapGeometryBuilder: 0x%llx\n", (uintptr_t)(Util::getGameInstance()->MapModule->MapGeometryBuilder) + 0x18);

		test++;
	}
	*/
}