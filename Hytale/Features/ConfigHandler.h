/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include <string>

namespace ConfigHandler {
	void LoadConfig(std::string name, bool inConfigDirectory);
	void SaveConfig(std::string name, bool inConfigDirectory);
	void CreateFishDirectory();
	bool FishDirectoryExists();
}