/*
 * Copyright (c) FishPlusPlus.
 */
#include "ChestESP.h"

#include "core.h"

ChestESP::ChestESP() : Feature("ChestESP") {
	this->radius   = this->RegisterSetting<SliderSetting>("Radius", 50.f, 5.f, 500.f);
	this->showName = this->RegisterSetting<ToggleSetting>("Show Name", false);
}

bool ChestESP::CanExecute() {
	if (!Util::IsValidPtr(Util::getGameInstance()))
		return false;
	return true;
}

void ChestESP::Initialize() {
	Util::log("Initialized ChestESP feature\n");
}
