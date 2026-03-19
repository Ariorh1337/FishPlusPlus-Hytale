/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include "Features/Feature.h"

class Nametags : public Feature {
public:
	Nametags() : Feature("Nametags") {};
private:
	bool CanExecute() override;
	void Initialize() override;
	void OnRender3D(Renderer3D& renderer3D);
};