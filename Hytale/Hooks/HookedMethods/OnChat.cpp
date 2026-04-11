/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"

#include "Features/ConfigHandler.h"

#pragma optimize("", off)
#pragma runtime_checks("", off)

void tpCommand(std::string command) {
    std::istringstream iss(command);
    float x;
    float y;
    float z;
    if (iss >> x >> y >> z) {
        HookData::queueTeleport = true;
        HookData::teleportTarget = Vector3(x, y, z);
    }
}

void configCommand(std::string command) {
    if (command.starts_with("save ")) {
        std::string name = command.substr(5);
        ConfigHandler::SaveConfig(name, true);
    }
    else if (command.starts_with("load ")) {
        std::string name = command.substr(5);
        ConfigHandler::LoadConfig(name, true);
	}
}

__declspec(safebuffers) __declspec(noinline)
void __fastcall Hooks::hkOnChat(uint64_t instance, HytaleString* chatString) {
    std::string message = chatString->getString();
    //Util::log("Chat message: %s\n", message.c_str());

    if (!message.starts_with('!')) {
        Hooks::oOnChat(instance, chatString);
        return;
    }

	std::string command = message.substr(1);

	if (command.starts_with("tp "))
        tpCommand(command.substr(3));

	if (command.starts_with("config "))
        configCommand(command.substr(7));

    if (command.starts_with("place ")) {
        std::string cmd = message.substr(6);
        std::istringstream iss(cmd);
        int x;
        int y;
        int z;
        int id;
        if (iss >> x >> y >> z >> id) {
            //Hooks::oSetClientBlock(Util::getGameInstance()->MapModule, x, y, z, (int)id, 0, 0, true);
        }
        
    }
        
        
}
#pragma runtime_checks("", restore)
#pragma optimize("", on)