/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "SettingButton.h"
#include "Features/Setting.h"
#include "sdk/ClientBlockType.h"

class BlockSelectionWindow;

class BlockSelectionButton : public SettingButton {
public:

	BlockSelectionButton(Setting<std::vector<ClientBlockType*>>* setting);
	void Render(double deltaTime) override;
	void Update(float mouseX, float mouseY) override;
	void MouseClicked(float mouseX, float mouseY, int vk) override;

private:

	BlockSelectionWindow* body;

	friend class BlockSelectionWindow;
};

class BlockSelectionWindow : public Component {
public:
	BlockSelectionWindow(BlockSelectionButton* parent);
	void Update(float mouseX, float mouseY) override;
	void Render(double deltaTime) override;

	bool open = false;

private:

	BlockSelectionButton* parent;

	friend class BlockSelectionButton;
};