#pragma once

#include "Structs/IndependentStructs.h"
#include <sdk/BaseDataTypes/Object.h>

struct UpdateBlockDamage : Object {
	BlockPosition blockPosition;
	float damage;
	float delta;
};