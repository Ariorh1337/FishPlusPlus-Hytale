/*
 * Copyright (c) FishPlusPlus.
 */
#include "Feature.h"

#include "../Renderer/Renderer3D.h"
#include "Settings/KeybindSetting.h"

Feature::Feature(std::string name) {
	m_name = name;
	m_category = "";
	active = false;
}

bool Feature::CanExecute() { return true; }
void Feature::OnActivate() {}
void Feature::OnDeactivate() {}
void Feature::Initialize() {}

void Feature::CreateForcedKeybind() {
	this->RegisterSetting<KeybindSetting>("Keybind", SDL_SCANCODE_UNKNOWN, this);
}

void Feature::setCategory(std::string category) {
	m_category = category;
}
