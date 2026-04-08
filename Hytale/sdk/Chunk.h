#pragma once
#include <cstdint>
#include "ChunkData.h"

struct Chunk {
	char pad_0x00[0x10];                                // 0x00
	void* lock;                                         // 0x10 System.Threading.Lock
	ChunkData* Data;									// 0x18 HytaleClient.Data.Map.ChunkData
	void* Rendered;										// 0x20 HytaleClient.Graphics.Map.RenderedChunk
	char pad_0x20[0x8];									// 0x28-0x30
	int Y;                                              // 0x30
	int a;                                              // 0x34
	int b;                                              // 0x38
	int c;                                              // 0x3c
	bool IsUnderground;									// 0x40
};