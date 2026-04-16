/*
 * Copyright (c) FishPlusPlus.
 *
 * SubTypeRegistry — caches MethodTable pointers for managed sub-types
 * (Position, Direction, BlockPosition, etc.) so they can be allocated
 * with RhpNewFast / RhpNewArray when building outgoing packets.
 *
 * Two sources fill the cache:
 *   - Initialize()  — statically seeded from known offsets at startup.
 *   - ScanPacket()  — learns from incoming packets at runtime.
 *     Once all offsets are confirmed in Initialize(), ScanPacket and the
 *     corresponding call in ProcessPacket.cpp can be removed entirely.
 */
#pragma once
#include "PacketFieldTable.h"
#include "sdk/BaseDataTypes/Object.h"

namespace SubTypeRegistry {

	void Initialize();

	// Scan a received packet and cache any previously unknown MethodTables.
	// REMOVABLE: delete once all offsets are confirmed in Initialize().
	void ScanPacket(Object* packet, PacketIndex index);

	void* GetMethodTable(const char* type_name);

	// Allocate a new managed object via RhpNewFast.
	void* Alloc(const char* type_name);

	// Allocate a new managed array via RhpNewArray.
	// Pass the full array type name, e.g. "Array<SyncInteractionChain*>".
	void* AllocArray(const char* type_name, int count);

	int CachedCount();

}
