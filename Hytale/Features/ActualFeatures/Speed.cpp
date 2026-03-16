/*
 * Copyright (c) FishPlusPlus.
 */
#include "Speed.h"
#include "core.h"

#include "Events/EventRegister.h"

Speed::Speed() : Feature("Speed") {
	this->speed = this->RegisterSetting<SliderSetting>("Speed", 1.0f, 0.0f, 5.0f);
}

bool Speed::CanExecute() {
	ValidPtrBool(Util::getLocalPlayer());
}

void Speed::Initialize() {
	Util::log("Initialized Speed feature");
	EventRegister::DoMoveCycleEvent.Subscribe([](DefaultMovementController* dmc, Vector3 dir) {
		Util::log("DoMoveCycle called with DMC: 0x%llX, offset: (%f, %f, %f)\n", dmc, dir.x, dir.y, dir.z);
	});
}