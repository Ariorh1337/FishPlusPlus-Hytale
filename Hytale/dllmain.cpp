/*
 * Copyright (c) FishPlusPlus.
 */
#include "Core.h"
#include "Hooks/Hooks.h"

#define GetSig(name, pattern) SM::name##Address = Util::PatternScan(pattern); \
Util::log("Found %s sig at: 0x%llX - 0x%llX = 0x%lX\n", #name, SM::name##Address, gameBase, (SM::name##Address - gameBase));\
if (!Util::IsValidPtr(SM::name##Address)) {                             \
    Util::log("Failed to get %s address\n", #name);                     \
    return false;                                                       \
}

#define GetSigByRef(name, pattern) SM::name##Address = Util::RelativeVirtualAddress(Util::PatternScan(pattern), 1, 5); \
Util::log("Found %s sig at: 0x%llX - 0x%llX = 0x%lX\n", #name, SM::name##Address, gameBase, (SM::name##Address - gameBase));\
if (!Util::IsValidPtr(SM::name##Address)) {                             \
    Util::log("Failed to get %s address\n", #name);                     \
    return false;                                                       \
}
/*// Available GC Registered Thread
void __fastcall GCThread(void* pArg) {
    while (!uninjecting) {
        Sleep(100);
    }
}*/


bool InitSigs() {
    GetSigByRef(DoMoveCycle, "E8 ? ? ? ? FF CD 75 ? 85 F6 0F 85 ? ? ? ? 48 8B 4B");
    GetSigByRef(HandleScreenShotting, "E8 ? ? ? ? 4C 8B 7D ? 49 8B 8F ? ? ? ? 39 09");
    GetSigByRef(OnUserInput, "E8 ? ? ? ? 48 8B 53 ? 48 8B 92 ? ? ? ? 38 12");
    GetSigByRef(SetCursorHidden, "E8 ? ? ? ? 0F B6 4B ? 85 C9 74");
    GetSigByRef(UpdateInputStates, "E8 ? ? ? ? 83 7E ? ? 75 ? 48 83 C4");
    GetSigByRef(OnChat, "E8 ? ? ? ? 48 8B 4D ? 48 8B 89 ? ? ? ? 48 8B 89");
    GetSigByRef(DrawScene, "E8 ? ? ? ? 80 7B ? ? 75 ? 48 89 5D");
    GetSigByRef(GCToEEInterface_CreateThread, "E8 ? ? ? ? 0F B6 C0 89 05 ? ? ? ? 85 C0");
    GetSigByRef(DrawEntityCharactersAndItems, "E8 ? ? ? ? 48 8B 4B ? 48 8B 49 ? BA ? ? ? ? 39 09 E8 ? ? ? ? 48 8B 85");
    
    SM::WglSwapBuffersAddress = (uint64_t) GetProcAddress(GetModuleHandleA("opengl32.dll"), "wglSwapBuffers");
    if (!Util::IsValidPtr(SM::WglSwapBuffersAddress)) {
        Util::log("Failed to get wglSwapBuffers address\n");
        return false;
    }
	Util::log("Finished initializing signatures\n");
    return true;
}

DWORD WINAPI startPoint(LPVOID lpParam) {
	Util::allocate_console();

    if (!InitSigs()){
        Util::log("Failed to Init Sigs\n");
        Util::free_console();
        return 0;
    }
    
    if (!Hooks::CreateHooks()) {
		Util::log("Failed to create hooks\n");
        Util::free_console();
        return 0;
    }
/*    // Available GC Registered Thread
    typedef void(__fastcall* ThreadStart)(void* pArg);
    bool success = ((bool(__fastcall*)(ThreadStart, void*, bool, const char*))SM::GCToEEInterface_CreateThreadAddress)(GCThread, nullptr, true, "HytaleInternal");*/
    
    
    return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
		dllBase = (uint64_t) hModule;
		gameBase = (uint64_t) GetModuleHandleA(0);

        MODULEINFO moduleInfo;
        if (GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(MODULEINFO))) {
            SIZE_T moduleSize = moduleInfo.SizeOfImage;
            dllBaseEnd = dllBase + moduleSize;
        }
        return startPoint(0x0);
        //CreateThread(nullptr, 0, startPoint, 0, 0, nullptr);
        //break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

