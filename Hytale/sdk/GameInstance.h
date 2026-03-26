/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Entity.h"
#include "Camera.h"
#include "Time.h"
#include "EntityStoreModule.h"
#include "InventoryModule.h"
#include "CameraModule.h"
#include "Engine.h"
#include "SceneRenderer.h"
#include "Chat.h"
#include "CharacterControllerModule.h"

class GameInstance
{
public:
	char pad_0000[16]; //0x0000
	Entity* Player; //0x0010
	Engine* Engine; //0x0018
	char pad_0020[8]; //0x0020
	Chat* Chat; //0x0028
	char pad_0030[72]; //0x0030
	SceneRenderer* SceneRenderer; //0x0078
	char pad_0078[136]; //0x0080
	Time* Time; //0x0108
	char pad_0108[8]; //0x0110
	StructBeforeCam* Camera; //0x0118
	char pad_0118[8]; //0x0120
	CharacterControllerModule* CharacterControllerModule; //0x0128
	CameraModule* CameraModule; //0x0130
	char pad_0130[8]; //0x0138
	EntityStoreModule* EntityStoreModule; //0x0140
	InventoryModule* InventoryModule; //0x0148
	void* InteractionModule; //0x0150
	char pad_0150[48]; //0x0158
	void* WeatherModule; //0x0188
	void* AmbienceFXModule; //0x0190
	char pad_0190[168]; //0x0198
	float ResolutionScaleMin; //0x0240
	float ResolutionScaleMax; //0x0244
	char pad_0240[52]; //0x0248
	float underwaterCausticsIntensity; //0x0274
	float underwaterCausticsScale; //0x0278
	float underwaterCausticsDistortion; //0x027C
	float cloudsUVMotionScale; //0x0280
	float cloudsUVMotionStrength; //0x0284
	float cloudsShadowsIntensity; //0x0288
	float cloudsShadowsScale; //0x028C
	float cloudsShadowsBlurriness; //0x0290
	float cloudsShadowsSpeed; //0x0294
	float UnderwaterBloomIntensity; //0x0298
	float UnderwaterBloomPower; //0x029C
	float DefaultBloomIntensity; //0x02A0
	float DefaultBloomPower; //0x02A4
	char pad_02A8[80]; //0x02A8
	Vector3 FoliageInteractionParams; //0x02F8
	char pad_0304[128]; //0x0304

};