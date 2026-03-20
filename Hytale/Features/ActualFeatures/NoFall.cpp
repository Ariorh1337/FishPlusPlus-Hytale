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
	if (!Util::IsValidPtr(Util::getLocalPlayer()))
		return false;
	if (!Util::IsValidPtr(Util::GetMovementController()))
		return false;
	return true;
}



void NoFall::Initialize() {
	Util::log("Initialized Nofall feature\n");
	RegisterEvent(this);
}