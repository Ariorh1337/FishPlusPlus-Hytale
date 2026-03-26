/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Math/Matrix4x4.h"

struct SceneRenderer {
	char pad[0x310];
	Matrix4x4 MPV;
};