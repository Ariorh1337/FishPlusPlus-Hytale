/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include "Features/Feature.h"

class ChestESP : public Feature {
public:
	ChestESP();
	bool CanExecute() override;
	void Initialize() override;
};
