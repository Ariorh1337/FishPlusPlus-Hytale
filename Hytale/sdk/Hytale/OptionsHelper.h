/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include "sdk/BaseDataTypes/HytaleString.h"

struct OptionsHelper {
	char pad_0000[24]; //0x0000
	HytaleString* UserDataDirectory; //0x0018
	char pad_0020[8]; //0x0020
	HytaleString* BaseGameDirectory; //0x0028
	HytaleString* JavaExecutable; //0x0030
};