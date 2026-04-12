/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "ICameraController.h"

struct CameraModule {
	char pad_0000[32]; //0x0000
	ICameraController* Controller; //0x0020
	char pad_0028[48]; //0x0028
	bool thirdPerson; //0x0058

};