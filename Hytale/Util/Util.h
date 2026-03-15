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

    bool WorldToScreen(Vector3 pos, Vector2& out);
    Matrix4x4 getViewProjMat();
    std::filesystem::path GetDirectory();
    SimpleTime HoursToTime(float hours);
    uint64_t BuildTicksFromHours(float hours);
    const char* GetKeyName(SDL_Scancode key);
    bool ShouldInteractWithGame();
    void allocate_console();
    void free_console();
    void log(const char* fmt, ...);
}