/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include <cstdint>
#include "BaseDataTypes/ConcurrentDictionary.h"
#include "ChunkColumn.h"
#include "ClientBlockType.h"

class GameInstance;
class Texture;

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
    char pad_0xA8[0x38];                                // 0xA8-0xE0 (primitive values)
    BlockPosition* _deletedBlockPositions;              // 0xE0 HytaleClient.InGame.Modules.Map.MapModule+BlockPosition[]
    uint8_t* _deletedBlockAge;                          // 0xE8 System.Byte[]
    uint16_t* UInt16Array3;                             // 0xF0 System.UInt16[]
    void* _commitedBlockPositionsFromCamera;            // 0xF8 System.Numerics.Vector3[]
    char pad_100[0x20];                                 // 0x100 padding
    ConcurrentDictionary<int64_t, ChunkColumn*>* _chunkColumns;   // 0x120 ConcurrentDictionary<long, ChunkColumn> _chunkColumns = new ConcurrentDictionary<long, ChunkColumn>();

    /*int WorldToChunk(int worldCoord) {
        return (worldCoord < 0) ? ((worldCoord + 1) / 32 - 1) : (worldCoord / 32);
	}

    int WorldToChunkCoord(int worldCoord, int chunk) {
        int result = worldCoord - (chunk * 32);
        return result < 0 ? result + 32 : result;
	}*/

    int32_t IndexOfWorldBlockInChunk(int x, int y, int z) {
        return ((y & 0x1F) << 10) | ((z & 0x1F) << 5) | (x & 0x1F);
    }

    int64_t IndexOfChunkColumn(int32_t x, int32_t z) {
        return ((int64_t) x << 32) | (z & 0xFFFFFFFFu); // Validated 
    }

    ChunkColumn* GetChunkColumn(int64_t key) {
        ChunkColumn* outValue = nullptr;
		bool found = _chunkColumns->TryGetValue(key, &outValue);
		return outValue;
	}

    ChunkColumn* GetChunkColumn(int worldChunkX, int worldChunkZ) {
        return GetChunkColumn(IndexOfChunkColumn(worldChunkX, worldChunkZ));
    }

    Chunk* GetChunk(int worldChunkX, int worldChunkY, int worldChunkZ) {
        ChunkColumn* column = GetChunkColumn(worldChunkX, worldChunkZ);
        if (!column)
			return nullptr;

		Array<Chunk*>* chunks = column->chunks;
        return chunks->get(worldChunkY);
    }

    int GetBlock(int worldX, int worldY, int worldZ, int undefinedBlockId) {
        int worldChunkX = worldX >> 5;
        int worldChunkY = worldY >> 5;
        int worldChunkZ = worldZ >> 5;
        Chunk* chunk = GetChunk(worldChunkX, worldChunkY, worldChunkZ);
		if (!chunk || !chunk->Data || !chunk->Data->Blocks)
            return undefinedBlockId;

		return chunk->Data->Blocks->GetBlockID(IndexOfWorldBlockInChunk(worldX, worldY, worldZ), undefinedBlockId);
    }

    ClientBlockType* GetBlock(int worldX, int worldY, int worldZ) {
		int blockId = GetBlock(worldX, worldY, worldZ, 1);
		return ClientBlockTypes->get(blockId);
    }

    ClientBlockType* GetBlock(Vector3 pos) {
        return GetBlock((int)std::floor(pos.x), (int)std::floor(pos.y), (int)std::floor(pos.z));
    }
};