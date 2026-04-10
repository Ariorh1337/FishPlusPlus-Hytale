/*
 * Copyright (c) FishPlusPlus.
 */
#include "BlockESP.h"

#include "core.h"

BlockESP::BlockESP() : Feature("BlockESP") {
	this->radius   = this->RegisterSetting<SliderSetting>("Radius", 50.f, 5.f, 500.f);
	this->showName = this->RegisterSetting<ToggleSetting>("Show Name", false);
}

bool BlockESP::CanExecute() {
	if (!Util::IsValidPtr(Util::getGameInstance()))
		return false;
	return true;
}

void BlockESP::Initialize() {
	Util::log("Initialized BlockESP feature\n");
}
