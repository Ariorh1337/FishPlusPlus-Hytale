#pragma once
#include <cstdint>
#include "PaletteChunkData.h"
#include "sdk/BaseDataTypes/Dictionary.h"

struct InteractionStateInfo {
	int BlockId;
	void* BlockType;
	float StateFrameTime;
	void* SoundEventReference;
};

struct BlockDamageStatus {
	int BlockIndex;
	float Timer;
};

struct ChunkData {
	void* m_table;										// 0x00
	PaletteChunkData* Blocks; 							// 0x08 HytaleClient.Data.Map.Chunk.PaletteChunkData
	Dictionary<int, InteractionStateInfo*>* CurrentInteractionStates; // 0x10 System.Collections.Generic.Dictionary`2[System.Int32,HytaleClient.Data.Map.ChunkData+InteractionStateInfo]
	Array<BlockDamageStatus*>* BlockHitTimers;			// 0x18 HytaleClient.Data.Map.ChunkData+BlockDamageStatus[]
};