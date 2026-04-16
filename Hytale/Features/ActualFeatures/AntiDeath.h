/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include "Features/Feature.h"

class AntiDeath : public Feature {
public:
	AntiDeath() : Feature("AntiDeath") {}

	bool CanExecute() override;
	void Initialize() override;
	void OnPacketRecieved(Object* packet, PacketIndex& index, bool& cancel);
};
