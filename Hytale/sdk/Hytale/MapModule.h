/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "sdk/BaseDataTypes/ConcurrentDictionary.h"
#include "ChunkColumn.h"
#include "ClientBlockType.h"
#include "Math/Color.h"

class GameInstance;
class Texture;

struct RenderBlockInfo {
	std::vector<int> BlockID;
	const char* DisplayName;
	Color BlockColor;
	int settingIndex = -1; // Optional index for user settings, default is -1 (no setting)
};

struct ChunkPosition {
	int X;
	int Y;
    int Z;
};

struct BlockPosition {
    int X;
    int Y;
    int Z;
};

class MapGeometryBuilder {
public:
    char pad_0x00[0x8];                                 // 0x00
    void* field_0x08;                                   // 0x08
    GameInstance* GameInstance;                         // 0x10 HytaleClient.InGame.GameInstance
    void* ChunkGeometryBuilder;                         // 0x18 HytaleClient.Graphics.Map.ChunkGeometryBuilder
    void* Thread;                                       // 0x20 System.Threading.Thread
    void* CancellationTokenSource1;                     // 0x28 System.Threading.CancellationTokenSource
    void* Lock1;                                        // 0x30 System.Threading.Lock
    void* BoundingFrustum;                              // 0x38 HytaleClient.Math.BoundingFrustum
    void* IntVector3HashSet;                            // 0x40 System.Collections.Generic.HashSet<IntVector3>
    void* AutoResetEvent1;                              // 0x48 System.Threading.AutoResetEvent
    void* AutoResetEvent2;                              // 0x50 System.Threading.AutoResetEvent
    void* MeshingRequestBlockingCollection;             // 0x58 System.Collections.Concurrent.BlockingCollection
    void* ThreadList;                                   // 0x60 System.Collections.Generic.List<Thread>
    void* ChunkBuildQueueDataArray;                     // 0x68 HytaleClient.InGame.Modules.Map.MapGeometryBuilder+ChunkBuildQueueData[]
    void* Lock2;                                        // 0x70 System.Threading.Lock
    void* ChunkBuildQueueDataList;                      // 0x78 System.Collections.Generic.List<ChunkBuildQueueData>
    void* Stopwatch;                                    // 0x80 System.Diagnostics.Stopwatch
    void* UInt16ArrayQueue1;                            // 0x88 System.Collections.Concurrent.ConcurrentQueue<UInt16[]>
    void* UInt16ArrayQueue2;                            // 0x90 System.Collections.Concurrent.ConcurrentQueue<UInt16[]>
    char pad_0x98[0x20];                                // 0x98-0xB8 (primitive values)
    void* CancellationTokenSource2;                     // 0xB8 System.Threading.CancellationTokenSource
	char pad_0xC0[0xC];                                 // 0xC0
	Vector3 CameraPosition;                             // 0xCC SOMEHOW chicken found camera position here
};

enum BitPackingType : int {
    Packed4Bit = 0,
    Packed8Bit = 1,
    Packed16Bit = 2
};

struct DBGBlockData {
    int32_t packedPos;
	Vector3 unpackedPos;
    int16_t BlockID;
	std::string blockName;
	BitPackingType packingType;
    uint64_t paletteAddress;
};

class MapModule {
public:
    char pad_0x00[0x8];                                 // 0x00
    void* field_0x08;                                   // 0x08
    GameInstance* GameInstance;                         // 0x10 HytaleClient.InGame.GameInstance
    Array<ClientBlockType*>* ClientBlockTypes;          // 0x18 HytaleClient.Data.Map.ClientBlockType[]
    void* BlockBreakingDecalDict;                       // 0x20 System.Collections.Generic.Dictionary
    Texture* Texture1;                                  // 0x28 HytaleClient.Graphics.Texture
    Texture* Texture2;                                  // 0x30 HytaleClient.Graphics.Texture
    void* ChunkColumnDict;                              // 0x38 System.Collections.Concurrent.ConcurrentDictionary
    uint32_t* UInt32Array;                              // 0x40 System.UInt32[]
    void* MapGeometryBuilder;                           // 0x48 HytaleClient.InGame.Modules.Map.MapGeometryBuilder
    float* FloatArray1;                                 // 0x50 System.Single[]
    Array<Chunk*>* _chunks;                             // 0x58 HytaleClient.InGame.Modules.Map.Chunk[]
    uint8_t* _drawMasks;                                // 0x60 System.Byte[]
    bool* _undergroundHints;                            // 0x68 System.Boolean[]
    void* Vector3Array1;                                // 0x70 System.Numerics.Vector3[]
    float* FloatArray2;                                 // 0x78 System.Single[]
    void* _boundingVolumes;                             // 0x80 HytaleClient.Math.BoundingBox[]
    uint16_t* UInt16Array1;                             // 0x88 System.UInt16[]
    uint16_t* UInt16Array2;                             // 0x90 System.UInt16[]
    ChunkPosition* _updatedChunksPositions;             // 0x98 HytaleClient.InGame.Modules.Map.MapModule+ChunkPosition[]
    void* ClientFluids;                                 // 0xA0 HytaleClient.Data.Map.ClientFluid[]
    char pad_0xA8[0x10];                                // 0xA8-0xB8 (primitive values)
	int renderedChunkCount;                             // 0xB8 int
	char pad_0xBC[0x24];                                // 0xBC
    BlockPosition* _deletedBlockPositions;              // 0xE0 HytaleClient.InGame.Modules.Map.MapModule+BlockPosition[]
    uint8_t* _deletedBlockAge;                          // 0xE8 System.Byte[]
    uint16_t* UInt16Array3;                             // 0xF0 System.UInt16[]
    void* _commitedBlockPositionsFromCamera;            // 0xF8 System.Numerics.Vector3[]
    char pad_100[0x20];                                 // 0x100 padding
    ConcurrentDictionary<int64_t, ChunkColumn*>* _chunkColumns;   // 0x120 ConcurrentDictionary<long, ChunkColumn> _chunkColumns = new ConcurrentDictionary<long, ChunkColumn>();

    int32_t IndexOfWorldBlockInChunk(int x, int y, int z) {
        return ((y & 0x1F) << 10) | ((z & 0x1F) << 5) | (x & 0x1F);
    }

    int64_t IndexOfChunkColumn(int32_t x, int32_t z) {
        return ((int64_t) x << 32) | (z & 0xFFFFFFFFu); // Validated 
    }

    int GetMaxViewDistance() {
        return renderedChunkCount * 32;
	}

    Chunk* GetChunk(int worldChunkX, int worldChunkY, int worldChunkZ) {
        ChunkColumn* column = nullptr;
        bool found = _chunkColumns->TryGetValue(IndexOfChunkColumn(worldChunkX, worldChunkZ), &column);
        if (!found)
			return nullptr;

		Array<Chunk*>* chunks = column->chunks;
        return chunks->get(worldChunkY);
    }

    int GetBlockID(int worldX, int worldY, int worldZ, int undefinedBlockId = 1) {
        int worldChunkX = worldX >> 5;
        int worldChunkY = worldY >> 5;
        int worldChunkZ = worldZ >> 5;
        Chunk* chunk = GetChunk(worldChunkX, worldChunkY, worldChunkZ);
		if (!chunk || !chunk->Data || !chunk->Data->Blocks)
            return undefinedBlockId;

		return chunk->Data->Blocks->GetBlockID(IndexOfWorldBlockInChunk(worldX, worldY, worldZ), undefinedBlockId);
    }

	std::vector<FishBlockData> FindBlocksInChunkColumn(int worldChunkX, int worldChunkZ, const std::vector<int>& targetBlockIds) {
		ChunkColumn* column = nullptr;
		bool found = _chunkColumns->TryGetValue(IndexOfChunkColumn(worldChunkX, worldChunkZ), &column);
		if (!found || !column->chunks)
			return {};

		std::vector<FishBlockData> results;
		for (int y = 0; y < column->chunks->count; y++) {
			Chunk* chunk = column->chunks->get(y);
			if (chunk && chunk->Data && chunk->Data->Blocks) {
				std::vector<FishBlockData> chunkResults = chunk->Data->Blocks->FindBlocks(targetBlockIds);
				for (FishBlockData& blockData : chunkResults) {
					// Convert local chunk coordinates to world coordinates
					int worldX = (worldChunkX << 5) + blockData.x;
					int worldY = (y << 5) + blockData.y;
					int worldZ = (worldChunkZ << 5) + blockData.z;
					results.emplace_back(worldX, worldY, worldZ, blockData.blockId);
				}
			}
		}
		return results;
	}

	// Scan a specific section of a single chunk (for incremental scanning)
	// localMinX/Y/Z and localMaxX/Y/Z are in chunk-local coordinates (0-31)
	std::vector<FishBlockData> FindBlocksInSingleChunk(int worldChunkX, int worldChunkY, int worldChunkZ, 
														 const std::vector<int>& targetBlockIds,
														 int localMinX = 0, int localMaxX = 31,
														 int localMinY = 0, int localMaxY = 31,
														 int localMinZ = 0, int localMaxZ = 31) {
		ChunkColumn* column = nullptr;
		bool found = _chunkColumns->TryGetValue(IndexOfChunkColumn(worldChunkX, worldChunkZ), &column);
		if (!found || !column->chunks)
			return {};

		if (worldChunkY >= column->chunks->count)
			return {};

		std::vector<FishBlockData> results;
		Chunk* chunk = column->chunks->get(worldChunkY);
		if (chunk && chunk->Data && chunk->Data->Blocks) {
			// Scan only the specified section of the chunk
			std::vector<FishBlockData> chunkResults = chunk->Data->Blocks->FindBlocksInSection(
				targetBlockIds, localMinX, localMaxX, localMinY, localMaxY, localMinZ, localMaxZ
			);
			for (FishBlockData& blockData : chunkResults) {
				// Convert local chunk coordinates to world coordinates
				int worldX = (worldChunkX << 5) + blockData.x;
				int worldY = (worldChunkY << 5) + blockData.y;
				int worldZ = (worldChunkZ << 5) + blockData.z;
				results.emplace_back(worldX, worldY, worldZ, blockData.blockId);
			}
		}
		return results;
	}


	ClientBlockType* GetBlockType(Vector3 pos) {
        int blockId = GetBlockID((int) pos.x, (int) pos.y, (int) pos.z, 1);
        return ClientBlockTypes->get(blockId);
    }

    DBGBlockData GetBlockData(int worldX, int worldY, int worldZ) {
        int worldChunkX = worldX >> 5;
        int worldChunkY = worldY >> 5;
        int worldChunkZ = worldZ >> 5;
        Chunk* chunk = GetChunk(worldChunkX, worldChunkY, worldChunkZ);

        DBGBlockData data;
        data.packedPos = IndexOfWorldBlockInChunk(worldX, worldY, worldZ);
        data.unpackedPos = Vector3(worldX, worldY, worldZ);
        data.BlockID = chunk->Data->Blocks->GetBlockID(data.packedPos, 1);
		data.blockName = ClientBlockTypes->get(data.BlockID)->Name->getString();
		data.packingType = (BitPackingType) chunk->Data->Blocks->GetPackingType();
		data.paletteAddress = (uint64_t) chunk->Data->Blocks->chunkSection->internalToExternal->list;
        return data;
	}
};