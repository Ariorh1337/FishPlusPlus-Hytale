/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"
#include "Events/EventRegister.h"

int teleportTicks = 0;

#pragma optimize("", off)
#pragma runtime_checks("", off)

__declspec(safebuffers) __declspec(noinline)
void __fastcall Hooks::hkDoMoveCycle(DefaultMovementController* dmc, Vector3 offset) {
    if (!initialized)
        return Hooks::oDoMoveCycle(dmc, offset);


    if (HookData::queueTeleport) {
        
        HookData::queueTeleport = false;
        teleportTicks = 5;
    }

    //doing it for 5 ticks makes it work? just 1 tick doesent work
    if (teleportTicks > 0) {
        Util::getLocalPlayer()->SetPositionTeleport(HookData::teleportTarget);
        teleportTicks--;
        return;
	}
	
    EventRegister::DoMoveCycleEvent.Invoke(dmc, offset);
    Hooks::oDoMoveCycle(dmc, offset);
}