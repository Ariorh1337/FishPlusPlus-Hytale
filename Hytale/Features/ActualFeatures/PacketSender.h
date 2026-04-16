/*
 * Copyright (c) FishPlusPlus.
 *
 * PacketSender — send any C2S packet from chat with full nested JSON support.
 */
#pragma once
#include "Features/Feature.h"
#include <string>

class PacketSender : public Feature {
public:
    PacketSender() : Feature("PacketSender") {};

    bool CanExecute() override;
    void Initialize() override;

    static bool TrySend(const std::string& json);
    static bool TryReceive(const std::string& json);
    static void DumpInteractions();
    static int  ResolveInteractionId(const std::string& name);
    static void ResolveNamesInJson(struct JsonVal& val);
    static void OpenPacketLabUI();

    static const char* GetPacketName(int index);
    static inline bool TracePackets = false;
};
