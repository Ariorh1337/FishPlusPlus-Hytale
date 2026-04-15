/*
 * Copyright (c) FishPlusPlus.
 *
 * SubTypeRegistry — caches MethodTable pointers for managed sub-types
 * (Position, Direction, BlockPosition, etc.) by scanning received packets.
 *
 * Usage:
 *   // In hkProcessPacket, after GetPacketIndex:
 *   SubTypeRegistry::ScanPacket(packet, index);
 *
 *   // In PacketSender, before allocating a sub-object:
 *   void* mt = SubTypeRegistry::GetMethodTable("Position");
 *   if (mt) { auto* pos = (Position*)RhpNewFast(mt); ... }
 */
#pragma once
#include "PacketFieldTable.h"
#include "sdk/BaseDataTypes/Object.h"

namespace SubTypeRegistry {

    void Initialize();

    // Scan a received packet's pointer fields and cache any new MethodTables.
    // Call this from hkProcessPacket for every incoming packet.
    void ScanPacket(Object* packet, PacketIndex index);

    // Returns the cached MethodTable for a type name, or nullptr if not seen yet.
    void* GetMethodTable(const char* type_name);

    // Allocate a new zeroed instance of a sub-type.
    // Returns nullptr if the MethodTable is not cached yet.
    void* Alloc(const char* type_name);

    // How many types have been cached so far (for debug logging).
    int CachedCount();

}  // namespace SubTypeRegistry
