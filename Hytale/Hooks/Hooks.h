/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "core.h"

#include "Math/Vector3.h"

namespace Hooks {

	typedef uint64_t(__fastcall* DoMoveCycle)(DefaultMovementController* dmc, Vector3* offset);
	inline DoMoveCycle oDoMoveCycle = nullptr;

	typedef uint64_t(__fastcall* SetUniformBuffers)(uint64_t thisptr);
	inline SetUniformBuffers oSetUniformBuffers = nullptr;

	typedef uint64_t(__fastcall* HandleScreenShotting)(App* app);
	inline HandleScreenShotting oHandleScreenShotting = nullptr;

	typedef uint64_t*(__fastcall* OnUserInput)(uint64_t a1, int* a2);
	inline OnUserInput oOnUserInput = nullptr;

	typedef void(__fastcall* SetCursorHidden)(Window* window, bool hidden);
	inline SetCursorHidden oSetCursorHidden = nullptr;

	typedef void(__fastcall* UpdateInputStates)(uint64_t thisptr, bool skipResetKeys);
	inline UpdateInputStates oUpdateInputStates = nullptr;

	typedef void(__fastcall* WeatherUpdate)(uint64_t thisptr, float deltaTime);
	inline WeatherUpdate oWeatherUpdate = nullptr;

	typedef void(__fastcall* SetActiveHotbarSlot)(uint64_t thisptr, unsigned int slot, bool triggerInteraction);
	inline SetActiveHotbarSlot oSetActiveHotbarSlot = nullptr;

	typedef void(__fastcall* OnChat)(uint64_t a1, uint64_t a2);
	inline OnChat oOnChat = nullptr;

	typedef uint64_t(__fastcall* DrawScene)(uint64_t a1);
	inline DrawScene oDrawScene = nullptr;

	bool CreateHooks();
	bool CreateNewHooks();
	bool CreateSafetyHooks();
	void UnhookAll();
}