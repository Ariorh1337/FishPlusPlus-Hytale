/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"

void __fastcall Hooks::hkHandleScreenShotting(App* app) {
    if (Util::app != app)
        Util::app = app;

    Hooks::oHandleScreenShotting(app);
    SDK::Main();
}