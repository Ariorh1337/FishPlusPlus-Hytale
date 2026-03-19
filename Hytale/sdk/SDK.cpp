/*
 * Copyright (c) FishPlusPlus.
 */
#include "SDK.h"

#include "Events/EventRegister.h"

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
			data.name = entity->Name->getName();
		else
			data.name = entityString->getName();

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
}