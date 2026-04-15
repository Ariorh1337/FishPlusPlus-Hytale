/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Features/Feature.h"

class ItemMagnet : public Feature {
public:
	ItemMagnet() : Feature("Item Magnet") {};

	void OnFrame();
	void Initialize() override;
};