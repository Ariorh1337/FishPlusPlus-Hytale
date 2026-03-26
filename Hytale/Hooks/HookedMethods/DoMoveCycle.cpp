#include "../Hooks.h"
#include "Events/EventRegister.h"

void __fastcall Hooks::hkDoMoveCycle(DefaultMovementController* dmc, Vector3 offset) {
    if (!initialized)
        return Hooks::oDoMoveCycle(dmc, offset);
	
    EventRegister::DoMoveCycleEvent.Invoke(dmc, offset);
    Hooks::oDoMoveCycle(dmc, offset);
}