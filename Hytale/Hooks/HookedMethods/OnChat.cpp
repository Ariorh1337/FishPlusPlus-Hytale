#include "../Hooks.h"

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
        GameInstance* instance = Util::getGameInstance();
        Entity* player = Util::getLocalPlayer();
        ValidPtrVoid(player);
        player->SetPositionTeleport(Vector3(x, y, z));
    }
}