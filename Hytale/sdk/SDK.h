/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include "Core.h"

struct EntityData {
	Entity* entityPtr;
	Entity::EntityType entityType;
	std::string name;
	Vector3 position;
	bool isLocalPlayer;
};

namespace SDK {
	inline std::mutex global_mutex;
	inline std::vector<EntityData> entities;
	inline bool initialized = false;


	extern void Main();
	extern void DoMoveCycleTest(DefaultMovementController* dmc, Vector3 dir);
}
