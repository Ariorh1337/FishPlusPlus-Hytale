/*
 * Copyright (c) FishPlusPlus.
 */
#include "NoFall.h"

#include "core.h"

void NoFall::OnMoveCycle(DefaultMovementController* dmc, Vector3& offset) {
	if (dmc->Velocity.y < -20.0f)
		dmc->Velocity.y = -20.0f;
}

bool NoFall::CanExecute() {
	ValidPtrBool(Util::getLocalPlayer());
	ValidPtrBool(Util::GetMovementController());
}



void NoFall::Initialize() {
	Util::log("Initialized Nofall feature\n");
	RegisterEvent(this);
}