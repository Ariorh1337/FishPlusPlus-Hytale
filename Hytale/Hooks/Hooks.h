/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "core.h"

#include "Math/Vector3.h"

#include "../Features/Speed.h"

namespace Hooks {

	typedef uint64_t(__fastcall* DoMoveCycle)(DefaultMovementController* dmc, Vector3* offset);
	inline static DoMoveCycle oDoMoveCycle = nullptr;

	typedef uint64_t(__fastcall* SetUniformBuffers)(uint64_t thisptr);
	inline static SetUniformBuffers oSetUniformBuffers = nullptr;

	typedef uint64_t(__fastcall* HandleScreenShotting)(App* app);
	inline static HandleScreenShotting oHandleScreenShotting = nullptr;

	typedef uint64_t*(__fastcall* OnUserInput)(uint64_t a1, int* a2);
	inline static OnUserInput oOnUserInput = nullptr;

	typedef void(__fastcall* SetCursorHidden)(Window* window, char hidden);
	inline static SetCursorHidden oSetCursorHidden = nullptr;

	typedef void(__fastcall* UpdateInputStates)(uint64_t thisptr, char skipResetKeys);
	inline static UpdateInputStates oUpdateInputStates = nullptr;

	typedef void(__fastcall* WeatherUpdate)(uint64_t thisptr, float deltaTime);
	inline static WeatherUpdate oWeatherUpdate = nullptr;

	typedef void(__fastcall* SetActiveHotbarSlot)(uint64_t thisptr, unsigned int slot, bool triggerInteraction);
	inline static SetActiveHotbarSlot oSetActiveHotbarSlot = nullptr;

	typedef void(__fastcall* OnChat)(uint64_t a1, uint64_t a2);
	inline static OnChat oOnChat = nullptr;

	typedef uint64_t(__fastcall* DrawScene)(uint64_t a1);
	inline static DrawScene oDrawScene = nullptr;

	bool CreateHooks();
	bool CreateNewHooks();
	bool CreateSafetyHooks();
	void UnhookAll();
}