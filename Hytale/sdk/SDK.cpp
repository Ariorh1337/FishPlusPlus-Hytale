/*
 * Copyright (c) FishPlusPlus.
 */
#include "SDK.h"

#include "Events/EventRegister.h"

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

		Entity::EntityPlayerType entityType = entity->entityPlayerType;

		EntityAssetStruct* assetStruct = entity->AssetNameStruct;
		ValidPtrLoop(assetStruct);

		HytaleString* entityString = assetStruct->entityString;
		ValidPtrLoop(assetStruct);

		EntityData data;
		data.entityPtr = entity;
		
		if (entityType == Entity::EntityPlayerType::Player)
			data.name = entity->Name->getString();
		else
			data.name = entityString->getString();

		data.entityType = entity->entityType;
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

void SDK::Main() {
	if (!initialized) {
		//EventRegister::DoMoveCycleEvent.Subscribe(DoMoveCycle);
		initialized = true;
	}

	if (Menu::isMenuOpen() && Util::app->Engine->Window->IsCursorHidden) {
		setCursorHidden(false);
	}

	if (!Menu::isMenuOpen() && Menu::m_justClosed) {
		UpdateInputStates(true);
		Menu::m_justClosed = false;
	}

	global_mutex.lock();
	entities = getEntities(Util::getLocalPlayer());
	global_mutex.unlock();

	static int test = 0;
	if (test < 1 && Util::app && Util::app->appInGame && Util::app->appInGame->gameInstance) {

		MapModule* mapModule = (MapModule*)Util::app->appInGame->gameInstance->MapModule;
		Vector3 playerPos = Util::getLocalPlayer()->Position;
		int block = mapModule->GetBlock((int)floor(playerPos.x), (int)floor(playerPos.y) - 1, (int)floor(playerPos.z), -67);
		Util::log("Block ID at player position: %i\n", block);

		test++;
	}
}