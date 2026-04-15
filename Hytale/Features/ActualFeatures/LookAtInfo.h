/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Features/Feature.h"
#include "Features/Settings/SliderSetting.h"

class LookAtInfo : public Feature {
public:
    LookAtInfo();
    void Initialize() override;
    bool CanExecute() override;
    bool AlwaysStartsDisabled() const override { return true; }

    float GetRange() const { return m_range->GetValue(); }

private:
    SliderSetting* m_range = nullptr;
};
