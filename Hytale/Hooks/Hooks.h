/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "core.h"

#include "Math/Vector3.h"

enum GCFlag : uint32_t {
	GCFlag_None = 0x0,
	GCFlag_SkipAddress = 0x10
};

struct GCData {
	char pad[0x18];            // 0x0
	uint64_t object;           // 0x18
	uint64_t regionStart;      // 0x20
	uint64_t regionEnd;        // 0x28
};

struct GCInstance {
	char pad1[0x8];            // 0x0
	GCData* gcData;            // 0x8
	uint64_t field_10;         // 0x10
	uint64_t address;          // 0x18
	char pad3[0x180];          // 0x20
	uint64_t field_1A0;        // 0x1A0
	char pad4[0x10];           // 0x1A8
	uint32_t flags;            // 0x1B8
};

namespace Hooks {
	typedef void(__fastcall* GCMethodLookup)(GCInstance* instance);
	inline static GCMethodLookup oGCMethodLookup = nullptr;

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

	typedef void(__fastcall* DrawScene)(GameInstance* a1);
	inline static DrawScene oDrawScene = nullptr;

	bool CreateHooks();
}