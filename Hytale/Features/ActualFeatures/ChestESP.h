/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include "Features/Feature.h"

#include "Features/Settings/SliderSetting.h"
#include "Features/Settings/ToggleSetting.h"

class ChestESP : public Feature {
public:
	ChestESP();
	bool CanExecute() override;
	void Initialize() override;

	SliderSetting* radius;
	ToggleSetting* showName;
};
