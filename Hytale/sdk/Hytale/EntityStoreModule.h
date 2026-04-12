/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

struct EntityStoreModule {
	char pad_0000[0x58]; // 0x0000
	Array<Entity*>* entityArray; // 0x0058
	char pad_0060[144]; //0x0060
	int entityCount; // 0x00F0
};