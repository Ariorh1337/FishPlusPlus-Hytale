#pragma once
#include <cstdint>
#include "Chunk.h"

struct ChunkColumn {
	char pad_0x00[0x10];                                // 0x00
	void* lock;                                         // 0x10 System.Threading.Lock
	Array<uint32_t>* tints;								// 0x18 System.UInt32[]
	Array<uint16_t>* Heights;							// 0x20 System.UInt16[]
	Array<Array<uint16_t>*>* Environments;				// 0x28 System.UInt16[][]
	Array<Chunk*>* chunks;								// 0x30 HytaleClient.InGame.Modules.Map.Chunk[]
};