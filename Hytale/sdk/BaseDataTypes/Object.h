#pragma once
#include "Util/Packet.h"
#include "sdk/BaseDataTypes/MethodTable.h"

struct Object {
	MethodTable* methodTable;
};