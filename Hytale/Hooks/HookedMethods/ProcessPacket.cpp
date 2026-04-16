/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"
#include "Events/EventRegister.h"
#include "Features/ActualFeatures/SubTypeRegistry.h"
#include "Features/ActualFeatures/PacketSender.h"

#pragma optimize("", off)
#pragma runtime_checks("", off)
void* Hooks::g_LastProcessPacketInstance = nullptr;

__declspec(safebuffers) __declspec(noinline)
void __fastcall Hooks::hkProcessPacket(void* instance, Object* packet) {
	Hooks::g_LastProcessPacketInstance = instance;
	
	bool cancel = false;
	PacketIndex index = GetPacketIndex(packet);
	
	if (PacketSender::TracePackets) {
		const char* name = PacketSender::GetPacketName(index);
		if (name) Util::log("[S2C] %s (%d)\n", name, (int)index);
		else Util::log("[S2C] Unknown Packet (%d)\n", (int)index);
	}

	SubTypeRegistry::ScanPacket(packet, index);
	EventRegister::PacketRecieveEvent.Invoke(packet, index, cancel);

	if (!cancel)
		Hooks::oProcessPacket(instance, packet);
}
#pragma runtime_checks("", restore)
#pragma optimize("", on)