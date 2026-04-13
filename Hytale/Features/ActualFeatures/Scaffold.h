/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Features/Feature.h"

#include "../Settings/SliderSetting.h"
#include "../Settings/MultiSetting.h"
#include "../Settings/KeybindSetting.h"

class Scaffold : public Feature {
public:
	Scaffold();
	void OnMoveCycle(DefaultMovementController* dmc, Vector3& offset);
	bool CanExecute() override;
	void Initialize() override;
};