#pragma once

#include "Structs/Object.h"
#include "Structs/IndependentStructs.h"

struct ClientPlaceBlockPacket : Object { // Struct from pEric
	void* mt;
	BlockPosition* position;
	BlockRotation* rotation;
	int placedBlockId;
	bool quickReplace;

	static void Send(Vector3 pos, int placedBlockId = 0, bool quickReplace = false) {
		ClientPlaceBlockPacket* packet = CreatePacket<ClientPlaceBlockPacket*>(ClientPlaceBlock_C2S);
		BlockPosition* position = new BlockPosition(pos.toFloor());
		BlockRotation* rotation = new BlockRotation();
		packet->position = position;
		packet->rotation = rotation;
		packet->placedBlockId = placedBlockId;
		packet->quickReplace = quickReplace;

		Packets::SendPacketImmediate(packet);

		delete position;
		delete rotation;
	}
};