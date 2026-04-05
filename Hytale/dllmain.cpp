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

#define GetMethodSigByRef(name, pattern) SM::name##Address = Util::RelativeVirtualAddress(Util::PatternScan(pattern), 1, 5); \
Util::log("Found Method %s sig at: 0x%llX - 0x%llX = 0x%lX\n", #name, SM::name##Address, gameBase, (SM::name##Address - gameBase));\
if (!Util::IsValidPtr(SM::name##Address)) {                             \
    Util::log("Failed to get %s address\n", #name);                     \
    return false;                                                       \
}

#define GetGlobalSigByRef(name, pattern) SM::name##Address = Util::RelativeVirtualAddress(Util::PatternScan(pattern), 3, 7); \
Util::log("Found Global %s sig at: 0x%llX - 0x%llX = 0x%lX\n", #name, SM::name##Address, gameBase, (SM::name##Address - gameBase));\
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
    GetMethodSigByRef(SetCursorHidden, "E8 ? ? ? ? 0F B6 4B ? 85 C9 74");
    GetMethodSigByRef(UpdateInputStates, "E8 ? ? ? ? 83 7E ? ? 75 ? 48 83 C4");
    GetMethodSigByRef(GCToEEInterface_CreateThread, "E8 ? ? ? ? 0F B6 C0 89 05 ? ? ? ? 85 C0");
    GetMethodSigByRef(beginGLContext, "E8 ? ? ? ? 8B 4D ? 44 8B 45 ? 8B 55 ? 41 FF D7");
    GetMethodSigByRef(endGLContext, "E8 ? ? ? ? 48 8B 75 ? 4C 8D 4E ? 48 8B 5D ? 48 8B 53 ? 44 3B 72");
    GetMethodSigByRef(renderQueueFlush, "E8 ? ? ? ? CC 57 56 55 53 48 83 EC ? 48 8B D9 48 8B F2 41 8B F8");
    GetMethodSigByRef(submitDrawCommands, "E8 ? ? ? ? E9 ? ? ? ? 90 90 90 90 90 90 48 83 79");

    GetGlobalSigByRef(g_UniformManager, "48 8B 0D ? ? ? ? 48 8B 49 ? 45 8B 07");
    GetGlobalSigByRef(g_BufferManager, "48 8B 05 ? ? ? ? 48 8B 40 ? 45 8B 51 ? 4C 8B B0 ? ? ? ? 44 8B CA");
    GetGlobalSigByRef(g_GlobalStateTable, "48 8D 05 ? ? ? ? 48 83 78 ? ? 0F 85 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 49 ? 45 8B 07");
    
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

    while (!(FindWindowA(nullptr, "Hytale"))) {
        Sleep(100);
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
        //return startPoint(0x0);
        CreateThread(nullptr, 0, startPoint, 0, 0, nullptr);
        //break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

