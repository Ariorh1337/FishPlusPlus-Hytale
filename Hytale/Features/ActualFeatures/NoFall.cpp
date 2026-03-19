/*
 * Copyright (c) FishPlusPlus.
 */
#include "NoFall.h"

#include "core.h"

void OnMoveCycle(DefaultMovementController* dmc, Vector3& offset) {
	if (dmc->Velocity.y < -20.0f)
		dmc->Velocity.y = -20.0f;
}

bool NoFall::CanExecute() {
	ValidPtrBool(Util::getLocalPlayer());
	ValidPtrBool(Util::GetMovementController());
}



void NoFall::Initialize() {
	Util::log("Initialized Nofall feature");
	EventRegister::DoMoveCycleEvent.Subscribe([&](DefaultMovementController* dmc, Vector3& dir) {
		if (this->IsActive())
			this->OnMoveCycle(dmc, dir);
		});
}