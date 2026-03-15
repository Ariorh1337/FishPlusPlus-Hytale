/*
 * Copyright (c) FishPlusPlus.
 */
#include "Speed.h"

#include <cmath>

#include "core.h"

Speed::Speed() : Feature("Speed") {
	this->speed = this->RegisterSetting<SliderSetting>("Speed", 1.0f, 0.0f, 5.0f);
}

bool Speed::CanExecute() {
	ValidPtrBool(Util::getLocalPlayer());
}