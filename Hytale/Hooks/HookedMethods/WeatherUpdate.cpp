/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"
#include "Features/FeatureHandler.h"
#include "Features/ActualFeatures/WorldModulate.h"

#pragma optimize("", off)
#pragma runtime_checks("", off)

__declspec(safebuffers) __declspec(noinline)
void __fastcall Hooks::hkWeatherUpdate(uint64_t instance, float deltaTime) {
    
    Hooks::oWeatherUpdate(instance, deltaTime);
    WorldModulate* worldModulate = static_cast<WorldModulate*>(FeatureHandler::GetFeatureFromName("WorldModulate"));
    if (!worldModulate)
        return;

    if (!worldModulate->IsActive())
        return;

    bool noFog = static_cast<ToggleSetting*>(worldModulate->GetSettingFromName("No Fog"))->GetValue();

    RecursiveSetting* fogChanger = static_cast<RecursiveSetting*>(worldModulate->GetSettingFromName("Fog Changer"));
    

    //TODO: Add the actual WeatherModule struct
    if (fogChanger->GetValue()) {
        float fogStart = static_cast<SliderSetting*>(fogChanger->GetSettingFromName("Start"))->GetValue();
        float fogEnd = static_cast<SliderSetting*>(fogChanger->GetSettingFromName("End"))->GetValue();
        Color color = static_cast<ColorSetting*>(fogChanger->GetSettingFromName("Color"))->GetValue();
        color = Color::Normalize(color);

        *(float*)((uintptr_t)(instance + 0x90)) = fogStart;
        *(float*)((uintptr_t)(instance + 0x94)) = fogEnd;
        *(float*)((uintptr_t)(instance + 0x110)) = color.r;
        *(float*)((uintptr_t)(instance + 0x114)) = color.g;
        *(float*)((uintptr_t)(instance + 0x118)) = color.b;
    }
    if (noFog)
        *(float*)((uintptr_t)(instance + 0x94)) = 0.0f;
}
#pragma runtime_checks("", restore)
#pragma optimize("", on)