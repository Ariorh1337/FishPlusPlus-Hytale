#include "../Hooks.h"

#pragma optimize("", off)
#pragma runtime_checks("", off)

static std::vector<int> allTargetBlockIds;

__declspec(safebuffers) __declspec(noinline)
void __fastcall Hooks::hkRebuildChunk(void* instance, ChunkColumn* a2, int chunkX, int chunkY, int chunkZ, int64_t a6, int64_t a7, int64_t a8, int64_t a9, int64_t a10, int64_t a11, int64_t a12, int a13, int a14, int64_t* a15) {
	Hooks::oRebuildChunk(instance, a2, chunkX, chunkY, chunkZ, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
	
	if (!Util::app)
		return;
	if (Util::app->Stage != AppStage::InGame)
		return;

	MapModule* mapModule = Util::getGameInstance()->MapModule;

	if (allTargetBlockIds.empty())
		allTargetBlockIds = mapModule->getAllImportantBlockIDs();
	if (allTargetBlockIds.empty())
		return;

	//Util::log("Rebuilt chunk at: %i, %i, %i\n", chunkX, chunkY, chunkZ);

	int num = 0x4A7;
	int num2 = 0;

	void** instanceArray = reinterpret_cast<void**>(instance);
	void* v95 = instanceArray[3];

	SDK::filteredBlockMutex.lock();
	SDK::filteredBlocks.erase(
		std::remove_if(
			SDK::filteredBlocks.begin(),
			SDK::filteredBlocks.end(),
			[&](const FilteredBlockResult& b) {
				int chunkPosX = b.position.x >= 0 ? b.position.x / 32 : (b.position.x - 31) / 32;
				int chunkPosY = b.position.y >= 0 ? b.position.y / 32 : (b.position.y - 31) / 32;
				int chunkPosZ = b.position.z >= 0 ? b.position.z / 32 : (b.position.z - 31) / 32;
				return chunkPosX == chunkX && chunkPosY == chunkY && chunkPosZ == chunkZ;
			}
		),
		SDK::filteredBlocks.end()
	);

	SDK::filteredBlockMutex.unlock();

	
	std::vector<FilteredBlockResult> newBlocks;

	for (int i = 0; i < 32; i++) {
		for (int j = 0; j < 32; j++) {
			for (int k = 0; k < 32; ++k) {
				int saveNum = num; 
				int blockId = *(uint32_t*)(a7 + 8 * num + 0x10);
				uintptr_t v95 = reinterpret_cast<uintptr_t>(reinterpret_cast<void**>(instance)[3]);
				ClientBlockType* clientBlock = *(ClientBlockType**)(v95 + 8 * blockId + 0x10);

				if ((int)(clientBlock + 0x161) == 0) { //drawtype
					num++;
					num2++;
					continue;
				}

				int index = num - 0x4A7;

				int size = 34;

				int x = index % size;
				int z = (index / size) % size;
				int y = index / (size * size);

				int worldX = chunkX * 32 + x;
				int worldY = chunkY * 32 + y;
				int worldZ = chunkZ * 32 + z;
				
			
				for (const auto& importantBlock : ImportantBlocks) {
					for (int id : importantBlock.BlockID) {
						if (blockId == id) {
							FilteredBlockResult result;
							result.position = Vector3((float)worldX, (float)worldY, (float)worldZ);
							result.blockId = blockId;
							result.displayName = importantBlock.DisplayName;
							result.color = importantBlock.BlockColor;
							newBlocks.push_back(result);
							break;
						}

					}

				}

				num++;
				num2++;
			}
			num += 2;
		}
		num += 68;
	}

	SDK::filteredBlockMutex.lock();
	SDK::filteredBlocks.insert(SDK::filteredBlocks.end(), newBlocks.begin(), newBlocks.end());
	SDK::filteredBlockMutex.unlock();
	
}

#pragma runtime_checks("", restore)
#pragma optimize("", on)