/*
 * Copyright (c) FishPlusPlus.
 */
#include "AntiDeath.h"

#include "core.h"
#include "sdk/Packets/PacketRegistry.h"
#include "sdk/BaseDataTypes/HytaleString.h"

bool AntiDeath::CanExecute() {
	return Util::isFullyInitialized();
}

void AntiDeath::Initialize() {
	Util::log("Initialized AntiDeath feature\n");
	RegisterEvent(this);
}

static bool StringFieldMatches(Object* packet, int offset, const char* value) {
	HytaleString* s = *(HytaleString**)((uint8_t*)packet + offset);
	if (!Util::IsValidPtr((uintptr_t)s))
		return false;
	return s->getString() == value;
}

void AntiDeath::OnPacketRecieved(Object* packet, PacketIndex& index, bool& cancel) {
	if (index == PlayAnimation_S2C) {
		// animation_id is a String* at +0x10
		if (StringFieldMatches(packet, 0x10, "Laydown") ||
			StringFieldMatches(packet, 0x10, "Sleep")   ||
			StringFieldMatches(packet, 0x10, "Death")) {
			cancel = true;
		}
		return;
	}

	if (index == CustomPage_S2C) {
		// key is a String* at +0x08 — block the respawn page
		if (StringFieldMatches(packet, 0x08, "com.hypixel.hytale.server.core.entity.entities.player.pages.RespawnPage")) {
			cancel = true;
		}
		return;
	}
}
