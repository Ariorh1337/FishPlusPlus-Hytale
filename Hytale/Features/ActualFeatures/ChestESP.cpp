/*
 * Copyright (c) FishPlusPlus.
 */
#include "ChestESP.h"

#include "core.h"

ChestESP::ChestESP() : Feature("ChestESP") {}

bool ChestESP::CanExecute() {
	if (!Util::IsValidPtr(Util::getGameInstance()))
		return false;
	return true;
}

void ChestESP::Initialize() {
	Util::log("Initialized ChestESP feature\n");
}
