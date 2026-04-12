#pragma once

#include "Math/BlockPosition.h"
#include "Math/BlockRotation.h"

struct ClientPlaceBlock {
	void* mt;
	BlockPosition* position;
	BlockRotation* rotation;
	int placedBlockId;
	bool quickReplace;
};