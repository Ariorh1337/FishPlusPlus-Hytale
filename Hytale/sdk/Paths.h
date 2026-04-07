/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include "HytaleString.h"

struct Paths {
	char pad_0000[8]; //0x0000
	HytaleString* ClientGameDirectory; //0x0008
	char pad_0010[16]; //0x0010
	HytaleString* ServerJar; //0x0020
};