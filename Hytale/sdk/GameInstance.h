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
#include "ChunkManager.h"

class GameInstance {
public:
	char pad_0[0x10];                                   //0x0
	Entity* Player;                                     //0x10
	App* App;                                           //0x18
	Engine* Engine;                                     //0x20
	Chat* Chat;                                         //0x28
	void* Notifications;                                //HytaleClient.InGame.Notifications
	void* HitDetection;                                 //HytaleClient.InGame.HitDetection
	void* VoiceManager;                                 //HytaleClient.Audio.Voice.VoiceManager
	void* unkown_atm_0;									
	void* QuicConnectionToServer;                       //HytaleClient.Networking.Connection.QuicConnectionToServer
	void* PacketHandler;                                //HytaleClient.Networking.PacketHandler
	void* Stopwatch;                                    //System.Diagnostics.Stopwatch
	void* List;                                         //System.Collections.Generic.List`1[System.ValueTuple`2[HytaleClient.Data.InputSystem.InputAction,System.Boolean]]
	void* some_String;                                  //System.String
	SceneRenderer* SceneRenderer;                       //0x78
	char pad_80[0x88];                                  //0x80
	Time* TimeModule;                                   //0x108
	void* AudioModule;                                  //0x110
	StructBeforeCam* MapModule;                         //0x118
	void* ItemLibraryModule;                            //0x120
	CharacterControllerModule* CharacterControllerModule; //0x128
	CameraModule* CameraModule;                         //0x130
	void* CollisionModule;                              //0x138
	EntityStoreModule* EntityStoreModule;               //0x140
	InventoryModule* InventoryModule;                   //0x148
	void* InteractionModule;                            //0x150 HytaleClient.InGame.Modules.Interaction.InteractionModule
	void* BuilderToolsModule;                           //0x158 HytaleClient.InGame.Modules.BuilderTools.BuilderToolsModule
	void* MachinimaModule;                              //0x160 HytaleClient.InGame.Modules.Machinima.MachinimaModule
	void* FXModule;                                     //0x168 HytaleClient.InGame.Modules.FXModule
	void* TrailStoreModule;                             //0x170 HytaleClient.InGame.Modules.Trails.TrailStoreModule
	void* ParticleSystemStoreModule;                    //0x178 HytaleClient.InGame.Modules.Particles.ParticleSystemStoreModule
	void* ScreenEffectStoreModule;                      //0x180 HytaleClient.InGame.Modules.ScreenEffectStoreModule
	void* WeatherModule;                                //0x188 HytaleClient.InGame.Modules.WeatherModule
	void* AmbienceFXModule;                             //0x190 HytaleClient.InGame.Modules.AmbienceFX.AmbienceFXModule
	void* DamageEffectModule;                           //0x198 HytaleClient.InGame.Modules.DamageEffectModule
	void* SleepModule;                                  //0x1A0 HytaleClient.InGame.Modules.Entities.SleepModule
	void* ClientFeatureModule;                          //0x1A8 HytaleClient.InGame.Modules.ClientFeatureModule
	void* DebugStatusBarModule;                         //0x1B0 HytaleClient.InGame.Modules.DebugStatusBarModule
	void* ProfilingModule;                              //0x1B8 HytaleClient.InGame.Modules.Profiling.ProfilingModule
	void* FpsOverlayModule;                             //0x1C0 HytaleClient.InGame.Modules.Fps.FpsOverlayModule
	void* AudioDebugModule;                             //0x1C8 HytaleClient.Audio.Debugger.AudioDebugModule
	void* ShortcutsModule;                              //0x1D0 HytaleClient.InGame.Modules.Shortcuts.ShortcutsModule
	void* InterfaceRenderPreviewModule;                 //0x1D8 HytaleClient.InGame.Modules.InterfaceRenderPreview.InterfaceRenderPreviewModul
	void* WorldMapModule;                               //0x1E0 HytaleClient.InGame.Modules.WorldMap.WorldMapModule
	void* DebugDisplayModule;                           //0x1E8 HytaleClient.InGame.Modules.DebugDisplayModule
	void* NetworkModule;                                //0x1F0 HytaleClient.InGame.Modules.NetworkModule
	void* MovementSoundModule;                          //0x1F8 HytaleClient.InGame.Modules.MovementSoundModule
	void* AutoCameraModule;                             //0x200 HytaleClient.InGame.Modules.AutoCameraModule
	void* DebugCommandsModule;                          //0x208 HytaleClient.InGame.Modules.DebugCommandsModule
	void* ConcurrentDictionary;                         //0x210 System.Collections.Concurrent.ConcurrentDictionary`2[System.String,System.String]
	char pad_218[0x28];                                 //0x218
	float ResolutionScaleMin;                           //0x240
	float ResolutionScaleMax;                           //0x244
	char pad_248[0x34];                                 //0x248
	float underwaterCausticsIntensity;                  //0x274
	float underwaterCausticsScale;                      //0x278
	float underwaterCausticsDistortion;                 //0x27C
	float cloudsUVMotionScale;                          //0x280
	float cloudsUVMotionStrength;                       //0x284
	float cloudsShadowsIntensity;                       //0x288
	float cloudsShadowsScale;                           //0x28C
	float cloudsShadowsBlurriness;                      //0x290
	float cloudsShadowsSpeed;                           //0x294
	float UnderwaterBloomIntensity;                     //0x298
	float UnderwaterBloomPower;                         //0x29C
	float DefaultBloomIntensity;                        //0x2A0
	float DefaultBloomPower;                            //0x2A4
	char pad_2A8[0x50];                                 //0x2A8
	Vector3 FoliageInteractionParams;                   //0x2F8
	char pad_304[0x80];                                 //0x304
};