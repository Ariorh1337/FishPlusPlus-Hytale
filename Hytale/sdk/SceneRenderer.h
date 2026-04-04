/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Math/Matrix4x4.h"

struct SceneRenderer {
    char pad_0000[0x30];        // 0x000

    void* renderDevice;         // 0x030 -> used for rdi (calls at +0x250, +0x3E0)

    char pad_0038[0x8];         // 0x038

    void* sceneContext;         // 0x038 -> *(+0x30) chain (rsi source)

    char pad_0040[0x30];        // 0x040

    void* drawList;             // 0x070 -> array base (0x90 stride)

    char pad_0078[0x60];        // 0x078

    void* filterTable;          // 0x0D8 -> used in filtered path

    char pad_00E0[0xD8];        // 0x0E0

    uint32_t drawCount;         // 0x1B8 -> loop limit

    char pad_01BC[0x50];        // 0x1BC

    uint32_t unk_20C;           // 0x20C
    uint32_t unk_210;           // 0x210
    uint32_t unk_214;           // 0x214

    char pad_0218[0xF8];        // 0x218

    Matrix4x4 MPV;              // 0x310
};