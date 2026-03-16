/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Features/Feature.h"

class NoFall : public Feature {
public:
	NoFall() : Feature("Nofall") {};

	//void PlayerMove(MoveCycleEvent& event) override;
	bool CanExecute() override;
	void Initialize() override;
};