/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include <string>
#include <Windows.h>

#include "Math/Vector3.h"
#include "Math/BoundingBox.h"

struct EntityAssetStruct {
	char pad[0x8];
	HytaleString* entityString;
};

class Entity
{
public:
	enum EntityType {
		None,
		Character,
		Item,
		Block
	};

	char pad_0000[24]; //0x0000
	void* GameInstance; //0x0018
	char pad_0020[32]; //0x0020
	bool N0000077D; //0x0040
	char pad_0041[87]; //0x0041
	EntityAssetStruct* AssetNameStruct; //0x0098
	char pad_00A0[96]; //0x00A0
	HytaleString* Name; //0x0100
	char pad_0108[192]; //0x0108
	int networkId; //0x01C8
	char pad_01CC[8]; //0x01CC
	EntityType entityType; // 0x01D4
	char pad_01D8[12]; //0x01D8
	float MoveProgress; //0x01E4
	char pad_01E8[12]; //0x01E8
	float Scale; //0x01F4
	char pad_01F8[52]; //0x01F8
	int isPlayer; //0x22C
	char pad_0230[16]; //0x0230
	float DisplayHealth; //0x0240
	char pad_0244[11]; //0x0244
	bool wasOnGround; //0x0246
	bool wasInFluid; //0x0247
	bool wasFalling; //0x0248
	bool wasJunping; //0x0249
	bool usable; //0x024A
	char pad_0254[24]; //0x0254
	Vector3 PreviousPosition; //0x026C
	Vector3 NextPos; //0x0278
	Vector3 Position; //0x0284
	Vector3 RenderPos; //0x0290
	BoundingBox Hitbox; //0x0294
	BoundingBox DefaultHitbox;
	BoundingBox CrouchedHitbox;
	BoundingBox IDKDefaultHitbox;
	BoundingBox IDKCrouchedHitbox;
	char pad_0314[208]; //0x0314
	float pitchRad; //0x03E4
	float yawRad; //0x03E8
	char pad_03EC[4]; //0x03EC
	float pitchRadOld; //0x03F0
	float yawRadOld; //0x03F4
	char pad_xxxx[4]; //0x03EC
	float renderPitchRad;
	float renderYawRad;
	char pad_0388[1268];

	void SetPositionTeleport(Vector3 nextPosition) {
		PreviousPosition = Position;
		NextPos = nextPosition;
		MoveProgress = 1.0f;
		Position = NextPos;
	}

	bool IsAPlayer() {
		return isPlayer == 1;
	}

}; //Size: 0x087C