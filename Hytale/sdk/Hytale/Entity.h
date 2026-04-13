/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include <string>
#include <Windows.h>

#include "Math/Vector3.h"
#include "Math/BoundingBox.h"
#include "ClientItemBase.h"

struct EntityAssetStruct {
	char pad[0x8];
	HytaleString* entityString;
};

class Entity {
public:
	enum EntityType {
		None,
		Character,
		Item,
		Block
	};

	char pad_0000[8];						//0x000
	void* UnknownPtr;						//0x008
	void* CombatTextArray;					//0x10 HytaleClient.InGame.Modules.Entities.Entity+CombatText[]
	void* GameInstance;						//0x18 HytaleClient.InGame.GameInstance
	char pad_0020[8];						//0x20 
	void* UniqueEntityEffectArray;			//0x28 HytaleClient.InGame.Modules.Entities.Entity+UniqueEntityEffect[]
	void* EntityEffectUpdateList;			//0x30 System.Collections.Generic.List`1[Hytale.Protocol.EntityEffectUpdate]
	void* ServerEffectEntryList;			//0x38 System.Collections.Generic.List`1[HytaleClient.InGame.Modules.Entities.Entity+ServerEffectEntry]
	void* EntityParticleList1;				//0x40 System.Collections.Generic.List`1[HytaleClient.InGame.Modules.Entities.Entity+EntityParticle]
	void* EntityParticleList2;				//0x48 System.Collections.Generic.List`1[HytaleClient.InGame.Modules.Entities.Entity+EntityParticle]
	void* EntityTrailList;					//0x50 System.Collections.Generic.List`1[HytaleClient.InGame.Modules.Entities.Entity+EntityTrail]
	void* EntityParticleList3;				//0x58 System.Collections.Generic.List`1[HytaleClient.InGame.Modules.Entities.Entity+EntityParticle]
	char pad_0060[24];						//0x60 
	void* DetailBoundingBoxDict;			//0x78 System.Collections.Generic.Dictionary`2[System.String,HytaleClient.InGame.Modules.Entities.Entity+DetailBoundingBox[]]
	void* ModelRenderer;					//0x80 HytaleClient.Graphics.BlockyModels.ModelRenderer
	void* EntityItemList;					//0x88 System.Collections.Generic.List`1[HytaleClient.InGame.Modules.Entities.Entity+EntityItem]
	void* ClientModelVFX;					//0x90 HytaleClient.InGame.Modules.Entities.ClientModelVFX
	EntityAssetStruct* AssetNameStruct;		//0x98 Hytale.Protocol.Model
	void* StringArray;						//0xA0 System.String[]
	void* BlockyModel;						//0xA8 HytaleClient.Data.BlockyModels.BlockyModel
	ClientItemBase* PrimaryItem;			//0xB0 HytaleClient.Data.Items.ClientItemBase
	ClientItemBase* SecondaryItem;			//0xB8 HytaleClient.Data.Items.ClientItemBase
	char pad_00C0[0x18];					//0xC0 
	void* CameraSettings; 					//0xD8 Hytale.Protocol.CameraSettings
	void* CameraSettings2; 					//0xE0 Hytale.Protocol.CameraSettings
	char pad_00E8[8]; 						//0xE8 
	void* Int32List; 						//0xF0 System.Collections.Generic.List`1[System.Int32]
	void* SoundEventReferenceArray; 		//0xF8 HytaleClient.InGame.Modules.Audio.AudioModule+SoundEventReference[]
	HytaleString* Name; 					//0x100 System.String
	void* AnimationSetDict; 				//0x108 System.Collections.Generic.Dictionary`2[System.String,HytaleClient.Data.ClientAnimationSet]
	void* StringPtr; 						//0x110 System.String
	void* EntityAnimation; 					//0x118 HytaleClient.InGame.Modules.Entities.EntityAnimation
	char pad_0120[8]; 						//0x120 
	void* EntityAnimation2; 				//0x128 HytaleClient.InGame.Modules.Entities.EntityAnimation
	char pad_0130[8]; 						//0x130 
	void* StringArray2; 					//0x138 System.String[]
	char pad_0140[8]; 						//0x140 
	void* SoundEventReferenceArray2;		//0x148 HytaleClient.InGame.Modules.Audio.AudioModule+SoundEventReference[]
	void* TextRenderer; 					//0x150 HytaleClient.Graphics.Fonts.TextRenderer
	char pad_0158[32]; 						//0x158 
	void* PlayerSkin; 						//0x178 Hytale.Protocol.PlayerSkin
	void* InteractionMetaStoreDict;			//0x180 System.Collections.Generic.Dictionary`2[...InteractionMetaStore]
	void* InteractionTypeDict; 				//0x188 System.Collections.Generic.Dictionary`2[Hytale.Protocol.InteractionType,System.Int32]
	void* Int32List2; 						//0x190 System.Collections.Generic.List`1[System.Int32]
	char pad_0198[8]; 						//0x198 
	void* ClientEntityStatValueArray;		//0x1A0 HytaleClient.Data.EntityStats.ClientEntityStatValue[]
	void* ClientEntityStatValueArray2;		//0x1A8 HytaleClient.Data.EntityStats.ClientEntityStatValue[]
	void* TimedStatUpdateList;				//0x1B0 System.Collections.Generic.List`1[...TimedStatUpdate]
	void* TimedStatUpdateList2;				//0x1B8 System.Collections.Generic.List`1[...TimedStatUpdate]
	char pad_01C0[8]; 						//0x1C0
	int networkId; 							//0x1C8
	char pad_01CC[8]; 						//0x1CC
	EntityType entityType; 					//0x1D4
	char pad_01D8[12]; 						//0x1D8
	float MoveProgress; 					//0x1E4
	char pad_01E8[12]; 						//0x1E8
	float Scale; 							//0x1F4
	char pad_01F8[52]; 						//0x1F8
	int isPlayer; 							//0x22C
	char pad_0230[16]; 						//0x230
	float DisplayHealth; 					//0x240
	char pad_0244[11]; 						//0x244
	bool wasOnGround; 						//0x246
	bool wasInFluid; 						//0x247
	bool wasFalling; 						//0x248
	bool wasJunping; 						//0x249
	bool usable; 							//0x24A
	char pad_0254[24]; 						//0x254
	Vector3 PreviousPosition; 				//0x26C
	Vector3 NextPos; 						//0x278
	Vector3 Position; 						//0x284
	Vector3 RenderPos; 						//0x290
	BoundingBox Hitbox; 					//0x294
	BoundingBox DefaultHitbox;
	BoundingBox CrouchedHitbox;
	BoundingBox IDKDefaultHitbox;
	BoundingBox IDKCrouchedHitbox;
	char pad_0314[208]; 					//0x314
	float pitchRad; 					//0x3E4
	float yawRad; 					//0x3E8
	char pad_03EC[4];					 //0x3EC
	float pitchRadOld; 					//0x3F0
	float yawRadOld; 					//0x3F4
	char pad_xxxx[4]; 					//0x3EC
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