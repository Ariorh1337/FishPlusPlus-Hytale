#pragma once

/*
* Chunk size: 32x32x32 = 32,768 blocks
* 
* blockData->count == 16384: 4-bit packing (2 blocks per byte, palette indices 0-15)
* blockData->count == 32768: 8-bit packing (1 block per byte, palette indices 0-255)
* blockData->count == 65536: 16-bit packing (1 block per 2 bytes, palette indices 0-65535)
* 
* Selection logic:
*   - If blockData->count == 16384: Use 4-bit extraction
*   - Else if internalToExternal->count > 256: Use 16-bit (short) packing
*   - Else: Use 8-bit (byte) packing
*/
struct IChunkData {
	void* m_table;										// 0x00
	void* field_0x8;									// 0x08
	Array<int>* internalToExternal;						// 0x10 System.Int32[]
	char pad_0x10[0x10];                                // 0x18-0x28

	bool isEmpty() {
		return internalToExternal == nullptr || internalToExternal->count == 0;
	}

	int GetBlockDataCount() {
		uint64_t arrayBase = *(uint64_t*)((uint64_t)this + 0x28);
		if (arrayBase == 0) return 0;
		return *(int*)(arrayBase + 0x8);
	}

	bool Is4BitPacked() {
		return GetBlockDataCount() == 16384;
	}
};

struct Abstract4BitPaletteChunkData : IChunkData {
	Array<uint8_t>* blockData;								// 0x28 System.Byte[] (4-bit packed, 2 per byte)
};

struct Abstract8BitPaletteChunkData : IChunkData {
	Array<uint8_t>* blockData;								// 0x28 System.Byte[] (1 per byte)
};

struct Abstract16BitPaletteChunkData : IChunkData {
	Array<uint16_t>* blockData;								// 0x28 System.UInt16[] (1 per short)
};

struct PaletteChunkData {
	void* m_table;										// 0x00
	IChunkData* chunkSection;							// 0x08

	int GetBlockID(int32_t idx, int undefinedBlockId) {
		if (chunkSection == nullptr || chunkSection->isEmpty())
			return undefinedBlockId;

		int blockDataCount = chunkSection->GetBlockDataCount();
		int paletteCount = chunkSection->internalToExternal->count;
		//printf("Getting block ID at index %d (blockData count: %d, palette count: %d)\n", idx, blockDataCount, paletteCount);
		int paletteIdx = 0;
		if (blockDataCount == 16384) {
			Abstract4BitPaletteChunkData* data4bit = (Abstract4BitPaletteChunkData*)chunkSection;
			uint8_t byteVal = data4bit->blockData->get(idx / 2);
			paletteIdx = (idx % 2 == 0) ? (byteVal & 0x0F) : ((byteVal >> 4) & 0x0F);
		} else {
			if (paletteCount > 256) {
				Abstract16BitPaletteChunkData* data16bit = (Abstract16BitPaletteChunkData*)chunkSection;
				paletteIdx = data16bit->blockData->get(idx);
			} else {
				Abstract8BitPaletteChunkData* data8bit = (Abstract8BitPaletteChunkData*)chunkSection;
				paletteIdx = data8bit->blockData->get(idx);
			}
		}

		if (paletteIdx >= paletteCount) {
			return undefinedBlockId;
		}

		return chunkSection->internalToExternal->get(paletteIdx);
	}
};