/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Math/Vector3.h"

struct ICameraController {
	char pad_0000[32]; //0x0000
	Vector3 PositionFirstPerson; //0x0020
	char pad_002C[60]; //0x002C
	// 0x5C float Distance from player to camera Left & Right (Read-Only)
	// 0x60 float Distance from player to camera Forward & Backward (Read-Only)
	Vector3 PositionThirdPerson; //0x0068
	// 0x74 float Offset Left & Right
	// 0x78 float Offset Height
	// 0x7C float Offset Forward & Backward
	// 0x80 float Pitch (Up & Down)
	// 0x84 float Yaw (Left & Right)
	// 0x88 float Roll (Tilt)
	char pad_0070[28]; //0x0070
	Vector3 _lookAt; //0x008C
	Vector3 _headLookAt; //0x0098
};