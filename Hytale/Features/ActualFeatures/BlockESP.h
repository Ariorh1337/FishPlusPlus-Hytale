/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include "Features/Feature.h"

#include "Features/Settings/SliderSetting.h"
#include "Features/Settings/ToggleSetting.h"

class BlockESP : public Feature {
public:
	BlockESP();
	bool CanExecute() override;
	void Initialize() override;

	SliderSetting* radius;
	ToggleSetting* showName;
};
