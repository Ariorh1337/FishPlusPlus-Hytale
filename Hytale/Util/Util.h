/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once


#include <filesystem>
#include <vector>

#include "../Math/Matrix4x4.h"
#include "../Math/Vector2.h"

#include "InputSystem.h"
#include "../sdk/Camera.h"
#include "../sdk/CameraModule.h"
#include "../sdk/GameInstance.h"
#include "../sdk/DefaultMovementController.h"
#include "../sdk/App.h"
#include "../sdk/Entity.h"

struct SimpleTime
{
    int hour;
    int minute;
    int second;
    int millisecond;
};

namespace Util {
    inline App* app;
    Entity* getLocalPlayer();
    GameInstance* getGameInstance();
    DefaultMovementController* GetMovementController();
    Camera* getCamera();
    CameraModule* getCameraModule();
    inline Matrix4x4 viewProjMat;
    inline Matrix4x4 orthoProjMat;
	inline bool orthoProjMatInitialized = false;

    inline float cursorPosX = 0;
    inline float cursorPosY = 0;
    inline bool console_allocated = false;
   
    double GetTime();
    bool IsValidPtr(void* ptr);
    bool IsValidPtr(uint64_t ptr);
    uint64_t RelativeVirtualAddress(uint64_t address, int opcode_size = 3, int opcode_length = 7);
    uint64_t PatternScan(const char* signature, const char* module = "HytaleClient.exe");

    bool WorldToScreen(Vector3 pos, Vector2& out);
    Matrix4x4 getViewProjMat();
    SimpleTime HoursToTime(float hours);
    uint64_t BuildTicksFromHours(float hours);
    const char* GetKeyName(SDL_Scancode key);
    bool ShouldInteractWithGame();
    bool isFullyInitialized();
    void allocate_console();
    void free_console();
    void log(const char* fmt, ...);
	HytaleString* ObjectToString(void* object);
}