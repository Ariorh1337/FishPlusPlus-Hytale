/*
 * Copyright (c) FishPlusPlus.
 */
#include "Core.h"
#include "Hooks/Hooks.h"

#define GetSig(name, pattern) SM::name##Address = PatternScan(pattern); \
if (!Util::IsValidPtr(SM::name##Address)) {                             \
    Util::log("Failed to get %s address\n", #name);                     \
    return false;                                                       \
}

bool InitSigs() {
    GetSig(DoMoveCycle, "55 41 57 41 56 41 55 41 54 57 56 53 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 44 0F 29 84 24 ? ? ? ? 48 8D AC 24 ? ? ? ? 33 C0 48 89 85 ? ? ? ? 0F 57 E4 48 B8");
    GetSig(HandleScreenShotting, "55 41 57 41 56 41 55 41 54 57 56 53 48 81 EC ? ? ? ? 48 8D AC 24 ? ? ? ? 33 C0 48 89 45 ? 0F 57 E4 0F 29 65 ? 48 89 45 ? 48 8B D9 48 8B 4B ? 48 8B 49");
    GetSig(OnUserInput, "41 57 41 56 41 55 41 54 57 56 55 53 48 83 EC ? 33 C0 48 89 44 24 ? 0F 57 E4 0F 29 64 24 ? 0F 29 64 24 ? 0F 29 64 24 ? 48 89 44 24 ? 48 8B D9 48 8B F2 8B 3E");
    GetSig(SetCursorHidden, "55 57 56 53 48 83 EC ? 48 8D 6C 24 ? 33 C0 48 89 45 ? 48 89 45 ? 48 8B D9 8B F2");
    GetSig(UpdateInputStates, "57 56 53 48 83 EC ? 48 8B D9 8B F2 48 8B 4B ? 48 85 C9 0F 84");
    GetSig(OnChat, "56 53 48 83 EC ? 48 8B F1 48 8B DA 38 1B 48 8B CB");


    SM::WglSwapBuffersAddress = (uint64_t) GetProcAddress(GetModuleHandleA("opengl32.dll"), "wglSwapBuffers");
    if (!Util::IsValidPtr(SM::WglSwapBuffersAddress)) {
        Util::log("Failed to get wglSwapBuffers address\n");
        return false;
    }
}

DWORD WINAPI startPoint(LPVOID lpParam) {
	Util::allocate_console();

    if (!InitSigs()){
        Util::log("Failed to Init Sigs\n");
        Util::free_console();
        return 0;
    }

    if (!Hooks::CreateSafetyHooks()) {
		Util::log("Failed to create hooks\n");
        Util::free_console();
        return 0;
    }

    while (true) {
        Sleep(100);

        if (GetAsyncKeyState(VK_END))
            break;
    }

    //MH_DisableHook(MH_ALL_HOOKS);
	Hooks::UnhookAll();
	Util::free_console();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
    Core::MODULEPTR = hModule;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(nullptr, 0, startPoint, 0, 0, nullptr);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

