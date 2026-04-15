/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"
#include "Events/EventRegister.h"
#include "Features/ActualFeatures/SubTypeRegistry.h"

#pragma optimize("", off)
#pragma runtime_checks("", off)
void* Hooks::g_LastProcessPacketInstance = nullptr;

__declspec(safebuffers) __declspec(noinline)
void __fastcall Hooks::hkProcessPacket(void* instance, Object* packet) {
	Hooks::g_LastProcessPacketInstance = instance;
	
	bool cancel = false;
	PacketIndex index = GetPacketIndex(packet);
	SubTypeRegistry::ScanPacket(packet, index);
	EventRegister::PacketRecieveEvent.Invoke(packet, index, cancel);

	if (!cancel)
		Hooks::oProcessPacket(instance, packet);
}
#pragma runtime_checks("", restore)
#pragma optimize("", on)