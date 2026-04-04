/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"

#pragma optimize("", off)
#pragma runtime_checks("", off)

__declspec(safebuffers) __declspec(noinline)
void __fastcall Hooks::hkHandleScreenShotting(App* app) {
    if (Util::app != app)
        Util::app = app;

    Hooks::oHandleScreenShotting(app);
    SDK::Main();
}