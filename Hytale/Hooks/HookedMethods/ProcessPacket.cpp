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
		if (index != Ping_S2C && index != UpdateTime_S2C && index != SetEntitySeed_S2C && index != EntityUpdates_S2C) {
			std::string json = PacketSender::PacketToJson(packet, index);
			if (!json.empty() && json != "{}")
				Util::log("[S2C] %s\n", json.c_str());
			else
				Util::log("[S2C] Unknown (%d)\n", (int)index);
		}
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