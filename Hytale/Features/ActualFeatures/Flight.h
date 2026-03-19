/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Features/Feature.h"

#include "../Settings/SliderSetting.h"
#include "../Settings/MultiSetting.h"
#include "../Settings/KeybindSetting.h"

class Flight : public Feature {
public:
	Flight();
private:
	void OnDeactivate() override;
	bool CanExecute() override;
	void Initialize() override;

	void OnMoveCycle(DefaultMovementController* dmc, Vector3& offset);

	MultiSetting* mode;
	SliderSetting* speed;
};