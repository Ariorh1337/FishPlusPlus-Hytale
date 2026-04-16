/*
 * Copyright (c) FishPlusPlus.
 *
 * PacketSender — build and dispatch any packet from a JSON description.
 *
 * JSON schema:
 *   { "name": "<PacketName>", "<field>": <value>, ... }
 *
 * Special values:
 *   "INTERACTION~<Name>"  — resolved to the matching interaction ID at send time
 *   "dump": true          — hex-dump the constructed packet before sending
 *
 * Chat commands:
 *   !packet-lab           — open the Packet Lab window
 *   !dump-interactions    — write all interaction IDs to a file
 */
#pragma once
#include "Features/Feature.h"
#include "sdk/BaseDataTypes/Object.h"
#include "sdk/Packets/PacketRegistry.h"
#include <string>

struct JsonVal;

class PacketSender : public Feature {
public:
	PacketSender() : Feature("PacketSender") {}

	bool CanExecute() override;
	void Initialize() override;

	static bool TrySend(const std::string& json);
	static bool TryReceive(const std::string& json);
	static void DumpInteractions();

	static int  ResolveInteractionId(const std::string& name);
	static void ResolveNamesInJson(JsonVal& val);

	static void        OpenPacketLabUI();
	static const char* GetPacketName(int index);

	static std::string PacketToJson(Object* pkt, PacketIndex index);

	static inline bool TracePackets = false;

	// chain_id counter offset within InteractionModule (GameInstance + 0x150).
	// AUTO~CHAIN_ID increments this directly. Verified on Hytale 0.5.x.
	static inline int ChainCounterOffset = 0xE8;
};
