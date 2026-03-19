/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include "Features/Feature.h"

class Nametags : public Feature {
public:
	Nametags() : Feature("Nametags") {};
	void OnRender3D(Renderer3D& renderer3D);
	bool CanExecute() override;
	void Initialize() override;
	
};