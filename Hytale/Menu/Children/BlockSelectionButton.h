/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include <Menu/Component.h>

#include <memory>

#include "SettingButton.h"
#include "Features/Setting.h"
#include <sdk/Hytale/ClientBlockType.h>


class BlockSelectionScreen;

class BlockSelectionButton : public SettingButton {
public:
	BlockSelectionButton(Setting<std::vector<ClientBlockType*>>* setting);

	void Render(double deltaTime) override;
	void Update(float mouseX, float mouseY) override;

	void MouseClicked(float mouseX, float mouseY, int vk);

	std::unique_ptr<BlockSelectionScreen> screen;
};

class BlockSelectionScreen : public Component {
	void Render(double deltaTime) override;
	void Update(float mouseX, float mouseY) override;

	void MouseClicked(float mouseX, float mouseY, int vk);
};