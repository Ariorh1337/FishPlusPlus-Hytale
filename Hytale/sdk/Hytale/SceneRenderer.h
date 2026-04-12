/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Math/Matrix4x4.h"
#include "RenderStructs.h"


struct SceneRenderer {
	char pad_0000[0x30];                    // 0x000

	RenderDevice* renderDevice;             // 0x030 -> contains RenderStats at +0x10

	SceneContextContainer* contextContainer;// 0x038 -> contains SceneContext* at +0x30

	char pad_0040[0x30];                    // 0x040

	EntityList* entityList;                 // 0x070 -> array of EntityDrawData (0x90 stride)

	char pad_0078[0x60];                    // 0x078

	OcclusionFilterTable* occlusionFilter;  // 0x0D8 -> visibility filter table

	char pad_00E0[0xD8];                    // 0x0E0

	uint32_t _entityDrawTaskCount;               // 0x1B8 -> number of entities to draw

	char pad_01BC[0x50];                    // 0x1BC

	uint32_t unk_EntityCount_20C;           // 0x20C
	uint32_t unk_EntityCount_210;           // 0x210
	uint32_t unk_EntityCount_214;           // 0x214

	char pad_0218[0xF8];                    // 0x218

	Matrix4x4 MPV;                          // 0x310

	int getTotalEntityCount() {
		return unk_EntityCount_20C + unk_EntityCount_210 + unk_EntityCount_214;
	}
};