/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Math/Matrix4x4.h"
#include <cstdint>

struct RenderDevice {
    char pad_0000[0x250];           // 0x000
    void* drawCallFn;               // 0x250
    char pad_0258[0x188];           // 0x258
    void* batchDrawFn;              // 0x3E0
    char pad_03E8[0x50];            // 0x3E8
    uint32_t drawCallCount;         // 0x438
    uint32_t vertexCount;           // 0x43C
};

struct SceneContext {
    char pad_0000[0x30];            // 0x000
    void* contextData;              // 0x030
};

struct ContextData {
    char pad_0000[0x74];            // 0x000
    uint32_t contextValue;          // 0x074
    char pad_0078[0x10];            // 0x078
    uint32_t shaderParam;           // 0x084
};

struct DrawListEntry {
    char pad_0000[0x74];            // 0x000
    uint32_t param_a;               // 0x074
    uint32_t param_b;               // 0x078
    uint16_t param_c;               // 0x07C
    char pad_007E[0x2];             // 0x07E
    uint32_t count;                 // 0x080
    char pad_0084[0x18];            // 0x084
    uint16_t filterIndex;           // 0x09C
};

struct DrawList {
    char pad_0000[0x8];             // 0x000
    uint32_t size;                  // 0x008
    char pad_000C[0x4];             // 0x00C
    DrawListEntry entries[1];       // 0x010 (variable size, stride 0x90)
};

struct FilterTable {
    char pad_0000[0x8];             // 0x000
    uint32_t size;                  // 0x008
    char pad_000C[0x4];             // 0x00C
    uint32_t filters[1];            // 0x010 (variable size)
};

struct SceneRenderer {
    char pad_0000[0x30];            // 0x000

    RenderDevice* renderDevice;     // 0x030

    SceneContext* sceneContext;     // 0x038

    char pad_0040[0x30];            // 0x040

    DrawList* drawList;             // 0x070

    char pad_0078[0x60];            // 0x078

    FilterTable* filterTable;       // 0x0D8

    char pad_00E0[0xD8];            // 0x0E0

    uint32_t drawCount;             // 0x1B8

    char pad_01BC[0x50];            // 0x1BC

    uint32_t unk_20C;               // 0x20C
    uint32_t unk_210;               // 0x210
    uint32_t unk_214;               // 0x214

    char pad_0218[0xF8];            // 0x218

    Matrix4x4 MPV;                  // 0x310
};