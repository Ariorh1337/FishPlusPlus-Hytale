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
		// TODO: replace with PacketSender::PacketToJson once field reader is implemented
		if (name) Util::log("[S2C] %s (%d)\n", name, (int)index);
		else       Util::log("[S2C] Unknown (%d)\n", (int)index);
	}

	// RUNTIME LEARNING: discovers unknown MethodTable offsets by scanning received packets.
	// Each newly found type is logged: [SubTypeReg] Learned 'X' offset=0xY (add to static table)
	// Once all needed offsets are confirmed in SubTypeRegistry::Initialize(), remove this call
	// and the corresponding runtime-learning functions in SubTypeRegistry.cpp.
	SubTypeRegistry::ScanPacket(packet, index);
	EventRegister::PacketRecieveEvent.Invoke(packet, index, cancel);

	if (!cancel)
		Hooks::oProcessPacket(instance, packet);
}
#pragma runtime_checks("", restore)
#pragma optimize("", on)