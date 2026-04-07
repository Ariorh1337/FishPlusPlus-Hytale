/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include "GameInstance.h"

struct AppInGame {
	char pad[0x10];
	GameInstance* gameInstance;
	char pad_0018[80]; //0x0018
	bool IsInPause; //0x0068
	char pad_0069[11]; //0x0069
	uint8_t Overlay; //0x0074
	char pad_0075[7]; //0x0075
};
