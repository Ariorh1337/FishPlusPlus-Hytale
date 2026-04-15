/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include <vector>

#include "../../Features/Feature.h"
#include "Menu/Children/BlockSelectionButton.h"
#include <sdk/Hytale/ClientBlockType.h>

class BlockSetting : public Setting<std::vector<ClientBlockType*>> {
public:
	BlockSetting(std::string name) : Setting(name, {}) {}

	std::unique_ptr<Component> CreateButton() override {
		auto btn = std::make_unique<BlockSelectionButton>(this);
		btn->setting = this;
		return btn;
	}
};