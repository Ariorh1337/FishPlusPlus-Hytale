/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"

#pragma optimize("", off)
#pragma runtime_checks("", off)

__declspec(safebuffers) __declspec(noinline)
void __fastcall Hooks::hkOnChat(uint64_t instance, HytaleString* chatString) {
    std::string message = chatString->getString();
    //Util::log("Chat message: %s\n", message.c_str());

    if (!message.starts_with('!')) {
        Hooks::oOnChat(instance, chatString);
        return;
    }

    std::istringstream iss(message.substr(1));
    float x;
    float y;
    float z;

    if (iss >> x >> y >> z) {
        HookData::queueTeleport = true;
		HookData::teleportTarget = Vector3(x, y, z);
    }
}
#pragma runtime_checks("", restore)
#pragma optimize("", on)