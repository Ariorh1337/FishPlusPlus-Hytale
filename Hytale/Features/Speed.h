/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "../FeatureDispatcher/FeatureDispatcher.h"

#include "FeatureDispatcher/Settings/SliderSetting.h"
#include "FeatureDispatcher/Settings/MultiSetting.h"

class Speed : public Feature {
public:
	Speed();
	float GetSpeed() { return this->speed->GetValue(); }
private:
	bool CanExecute() override;
	SliderSetting* speed;
};