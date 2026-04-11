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
	void OnRender3D(Renderer3D& renderer3D);
	bool CanExecute() override;
	void Initialize() override;

	SliderSetting* radius;
	ToggleSetting* showName;
};
