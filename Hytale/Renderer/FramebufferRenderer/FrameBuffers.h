/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include <memory>

#include "OutlineFboRenderer.h"

class FrameBuffers {
public:
	inline static std::unique_ptr<OutlineFboRenderer> entityOutlineFBO = nullptr;
	inline static std::unique_ptr<OutlineFboRenderer> itemOutlineFBO = nullptr;

	static void initFBOS() {
		entityOutlineFBO = std::make_unique<OutlineFboRenderer>();
		itemOutlineFBO = std::make_unique<OutlineFboRenderer>();
	}

	static void resizeAll(int width, int height) {
		entityOutlineFBO->resize(width, height);
		itemOutlineFBO->resize(width, height);
	}
};