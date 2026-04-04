/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"
#include "Events/EventRegister.h"

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

    fboRenderer->draw();
}