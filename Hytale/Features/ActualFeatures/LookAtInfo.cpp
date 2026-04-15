/*
 * Copyright (c) FishPlusPlus.
 */
#include "LookAtInfo.h"
#include "Core.h"

LookAtInfo::LookAtInfo() : Feature("LookAtInfo") {
    m_range = RegisterSetting<SliderSetting>("Range", 8.0f, 1.0f, 64.0f);
}

void LookAtInfo::Initialize() {
    Util::log("Initialized LookAtInfo feature\n");
}

bool LookAtInfo::CanExecute() {
    return Util::isFullyInitialized();
}
