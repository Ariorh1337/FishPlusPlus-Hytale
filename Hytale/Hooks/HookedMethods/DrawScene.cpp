/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"
#include "Events/EventRegister.h"

#include "Renderer/FrameBufferRenderer/FrameBuffers.h"

#include "Features/FeatureHandler.h"
#include "Features/ActualFeatures/Outline.h"

#pragma optimize("", off)
#pragma runtime_checks("", off)

__declspec(safebuffers) __declspec(noinline)
void __fastcall Hooks::hkDrawScene(GameInstance* instance) {
    Hooks::oDrawScene(instance);
    if (!initialized)
        return;

    Renderer3D renderer3D;
    EventRegister::Render3DEvent.Invoke(renderer3D);
    renderer3D.Render();

    /*
    if (Util::app->Stage == AppStage::InGame) {
        Vector3 playerPos = Util::getLocalPlayer()->Position;
        Vector3 renderPos((int)std::floor(playerPos.x), (int)std::floor(playerPos.y) - 1, (int)std::floor(playerPos.z));
        ClientBlockType* block = Util::getGameInstance()->MapModule->GetBlock(renderPos);
        renderer3D.BoxLines(Vector3((int)std::floor(renderPos.x), (int)std::floor(renderPos.y), (int)std::floor(renderPos.z)), Vector3(1, 1, 1), Color::Normalize(255, 0, 0, 50), Color::Normalize(Color::Red()));
        Vector2 screenPos;
        if (Util::WorldToScreen(Vector3(renderPos.x + 0.5f, renderPos.y + 0.5f, renderPos.z + 0.5f), screenPos))
		    Fonts::Figtree->RenderText(block->Name->getString(), screenPos.x, screenPos.y, 1, Color::White());
    }
    */

    

    Outline* outline = static_cast<Outline*>(FeatureHandler::GetFeatureFromName("Outline"));
    

    if (outline->IsActive() && outline->CanExecute()) {
        RecursiveSetting* entities = static_cast<RecursiveSetting*>(outline->GetSettingFromName("Entities"));
        if (entities->GetValue()) {
            bool glow = static_cast<ToggleSetting*>(entities->GetSettingFromName("Glow"))->GetValue();
            float glowSize = static_cast<SliderSetting*>(entities->GetSettingFromName("Glow Size"))->GetValue();
            Color color = static_cast<ColorSetting*>(entities->GetSettingFromName("Color"))->GetValue();

            OutlineFboRenderer::OutlineUniforms uniforms(Color::Normalize(color), glow, (int)glowSize);
            FrameBuffers::entityOutlineFBO->setupPass(uniforms);
            FrameBuffers::entityOutlineFBO->draw();
        }
        RecursiveSetting* items = static_cast<RecursiveSetting*>(outline->GetSettingFromName("Items"));
        if (items->GetValue()) {
            bool glow = static_cast<ToggleSetting*>(items->GetSettingFromName("Glow"))->GetValue();
            float glowSize = static_cast<SliderSetting*>(items->GetSettingFromName("Glow Size"))->GetValue();
            Color color = static_cast<ColorSetting*>(items->GetSettingFromName("Color"))->GetValue();

            OutlineFboRenderer::OutlineUniforms uniforms(Color::Normalize(color), glow, (int)glowSize);
            FrameBuffers::itemOutlineFBO->setupPass(uniforms);
            FrameBuffers::itemOutlineFBO->draw();
        }
    }
}
#pragma runtime_checks("", restore)
#pragma optimize("", on)