/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "core.h"

#include "Math/Vector3.h"

namespace Hooks {

	typedef void(__fastcall* DoMoveCycle)(DefaultMovementController* dmc, Vector3 offset);
	inline static DoMoveCycle oDoMoveCycle = nullptr;

	typedef void(__fastcall* HandleScreenShotting)(App* app);
	inline static HandleScreenShotting oHandleScreenShotting = nullptr;

	typedef void(__fastcall* OnUserInput)(uint64_t a1, SDL_Event a2);
	inline static OnUserInput oOnUserInput = nullptr;

	typedef void(__fastcall* SetCursorHidden)(Window* window, bool hidden);
	inline static SetCursorHidden oSetCursorHidden = nullptr;

	typedef void(__fastcall* UpdateInputStates)(uint64_t thisptr, bool skipResetKeys);
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