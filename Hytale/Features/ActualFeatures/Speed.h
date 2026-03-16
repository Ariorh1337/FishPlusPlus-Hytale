/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Features/Feature.h"

#include "Features/Settings/SliderSetting.h"
#include "Features/Settings/MultiSetting.h"

class Speed : public Feature {
public:
	Speed();
	float GetSpeed() { return this->speed->GetValue(); }
private:
	bool CanExecute() override;
	void Initialize() override;
	SliderSetting* speed;

};