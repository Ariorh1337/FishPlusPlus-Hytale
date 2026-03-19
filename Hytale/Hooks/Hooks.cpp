/*
 * Copyright (c) FishPlusPlus.
 */
#include "Core.h"
#include "Hooks.h"
#include "external/safetyhook/safetyhook.hpp"
#include "Events/EventRegister.h"
#include "Features/FeatureHandler.h"

static bool initialized = false;
static bool initialized3D = false;

inline SafetyHookInline shWglSwapBuffers{ };
inline SafetyHookInline shDoMoveCycle{ };
inline SafetyHookInline shHandleScreenShotting{ };
inline SafetyHookInline shOnUserInput{ };
inline SafetyHookInline shSetCursorHidden{ };
inline SafetyHookInline shOnChat{ };

static std::unique_ptr<Menu> menu;


#define CREATE_HOOK(name) \
if (MH_CreateHook((LPVOID)SM::name##Address, &H##name, reinterpret_cast<LPVOID*>(&o##name)) != MH_OK) {\
    Util::log("Failed to hook %s\n", #name);\
    return false;\
}\

#define CREATE_SIG_HOOK(name, pattern) \
std::uintptr_t name##Address = Util::PatternScan(pattern);\
if (MH_CreateHook((LPVOID)name##Address, &H##name, reinterpret_cast<LPVOID*>(&o##name)) != MH_OK) {\
    Util::log("Failed to hook %s\n", #name);\
    return false;\
}\

static void* GetAnyGLFuncAddress(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == nullptr || p == (void*)0x1 || p == (void*)0x2 ||
        p == (void*)0x3 || p == (void*)-1) {
		static HMODULE module = GetModuleHandleA("opengl32.dll");
        if (!module)
            module = LoadLibraryA("opengl32.dll");

        p = (void*)GetProcAddress(module, name);
    }
    return p;
}

typedef BOOL(WINAPI* WglSwapBuffers)(HDC hdc);
WglSwapBuffers oWglSwapBuffers = nullptr;
BOOL WINAPI HWglSwapBuffers(HDC hdc) {
    if (!initialized) {
        if (!gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }
        Shaders::initShaders();
        Fonts::initFonts();

        Renderer2D::InitRenderer();

        menu = std::make_unique<Menu>(hdc);

		FeatureHandler::Init();

        initialized = true;
    }

    static double lastTime = 0.0;
    double currentTime = Util::GetTime();
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    HWND hwnd = WindowFromDC(hdc);
    RECT r;
    POINT current;
    GetCursorPos(&current);
    ScreenToClient(hwnd, &current);
    Util::cursorPosX = current.x;
    Util::cursorPosY = current.y;

    if (Util::IsValidPtr(Util::app)) {
        if (!Util::orthoProjMatInitialized) {
            Util::orthoProjMat = Matrix4x4::Orthographic(0.0f, Util::app->Engine->Window->WindowWidth, Util::app->Engine->Window->WindowHeight, 0.0f, -1.0f, 1.0f);
            Util::orthoProjMatInitialized = true;
        }

        Renderer3D renderer3D;
        EventRegister::Render3DEvent.Invoke(renderer3D);
        renderer3D.Render();


        Fonts::Figtree->RenderText(std::format("App: 0x{:x}", reinterpret_cast<uintptr_t>(Util::app)), 0.0f, 10.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("AppInGame: 0x{:x}", reinterpret_cast<uintptr_t>(Util::app->appInGame)), 0.0f, 20.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("GameInstance: 0x{:x}", reinterpret_cast<uintptr_t>(Util::getGameInstance())), 0.0f, 30.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("LocalPlayer: 0x{:x}", reinterpret_cast<uintptr_t>(Util::getLocalPlayer())), 0.0f, 40.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("DMC: 0x{:x}", reinterpret_cast<uintptr_t>(Util::GetMovementController())), 0.0f, 50.0f, 0.5f, Color::White());

        Fonts::Figtree->RenderText(std::format("Fish++ Hytale by LimitlessChicken aka milaq", reinterpret_cast<uintptr_t>(Util::app)), 500.0f, 10.0f, 0.5f, Color::White());

        menu->Run(deltaTime);
    }

    InputSystem::keysPressed.clear();
    InputSystem::keysDepressed.clear();

    return oWglSwapBuffers(hdc);
}

void __fastcall HDoMoveCycle(DefaultMovementController* dmc, Vector3 offset) {
	EventRegister::DoMoveCycleEvent.Invoke(dmc, offset);
	return Hooks::oDoMoveCycle(dmc, offset);
}

void __fastcall HHandleScreenShotting(App* app) {
    if (Util::app != app)
        Util::app = app;

    SDK::Main();

	if (Hooks::oHandleScreenShotting)
        Hooks::oHandleScreenShotting(app);
}


void __fastcall HOnUserInput(uint64_t thisptr, SDL_Event a2) {
    if (a2.type != SDL_KEYDOWN && a2.type != SDL_KEYUP) {
        Hooks::oOnUserInput(thisptr, a2);
        return;
    }

    SDL_Scancode key = a2.key.scancode;
	if (a2.type == SDL_KEYDOWN) {
		if (a2.key.repeat) {
            Hooks::oOnUserInput(thisptr, a2);
            return;
        }

        InputSystem::keysPressed.insert(key);
        InputSystem::keysHeld.insert(key);
        InputSystem::keysUnheld.erase(key);
    }
    else if (a2.type == SDL_KEYUP) {
        InputSystem::keysHeld.erase(key);
        InputSystem::keysUnheld.insert(key);
        InputSystem::keysDepressed.insert(key);
    }
    Hooks::oOnUserInput(thisptr, a2);
    return;
}

void __fastcall HOnChat(uint64_t thisptr, uint64_t a2) {
    try {
        HytaleString* stringTest = (HytaleString*) a2;
		//Util::log("Chat message: %s\n", stringTest->getName().c_str());

        if (!stringTest->getName().starts_with('!')) {
            Hooks::oOnChat(thisptr, a2);
            return;
        }

        std::string message = stringTest->getName();

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
    catch (...) {
        std::cout << "Exception in OnChat\n";
	}
	if (Hooks::oOnChat)
        Hooks::oOnChat(thisptr, a2);
}

uint64_t __fastcall HDrawScene(uint64_t thisptr) {

    Renderer3D renderer3D;
    EventRegister::Render3DEvent.Invoke(renderer3D);
    renderer3D.Render();

    return Hooks::oDrawScene(thisptr);
}

void HGCMethodLookup(GCInstance* instance) {
    if ((instance->flags & 0x10) == 0) {
        static uint64_t getGcObject;
        if (!getGcObject)
            getGcObject = Util::RelativeVirtualAddress(Util::PatternScan("E8 ? ? ? ? 48 89 06 4C 8B FE"), 1, 5);

        if (!((uint64_t(__fastcall*)(GCData*, uint64_t))getGcObject)(instance->gcData, instance->address))
            instance->flags |= GCFlag::GCFlag_SkipAddress;
    }
    Hooks::oGCMethodLookup(instance);
}

bool Hooks::CreateHooks() {
    Util::log("Creating Hooks\n");
    if (MH_Initialize() != MH_OK) {
		Util::log("Failed to initialize MinHook");
        return false;
    }

    CREATE_SIG_HOOK(GCMethodLookup, "40 53 57 48 83 EC ? 48 8D B9"); //E8 ? ? ? ? 83 3D ? ? ? ? ? 72 ? 49 8B CE   
    CREATE_HOOK(WglSwapBuffers);
    CREATE_HOOK(DoMoveCycle);
    CREATE_HOOK(HandleScreenShotting);
    CREATE_HOOK(OnUserInput);
    CREATE_HOOK(OnChat);
    //CREATE_HOOK(SetCursorHidden, "55 57 56 53 48 83 EC ? 48 8D 6C 24 ? 33 C0 48 89 45 ? 48 89 45 ? 48 8B D9 8B F2");

    //CREATE_HOOK(UpdateInputStates, "57 56 53 48 83 EC ? 48 8B D9 8B F2 48 8B 4B ? 48 85 C9 0F 84");
    //CREATE_HOOK(SetActiveHotbarSlot, "55 41 56 57 56 53 48 83 EC ? 48 8D 6C 24 ? 48 8B D9 8B F2 48 83 7B");
    //CREATE_HOOK(WeatherUpdate, "57 56 55 53 48 83 EC ? 0F 29 74 24 ? 48 8B D9 48 8B F2 48 8B 4B ? 48 8B 89 ? ? ? ? 48 8B 79 ? 80 BB ? ? ? ? ? 74 ? 80 7B ? ? 0F 85 ? ? ? ? 48 8B CF 4C 8D 1D ? ? ? ? 41 FF 13 85 C0 0F 85 ? ? ? ? 0F B6 83 ? ? ? ? 88 83 ? ? ? ? F3 0F 10 76 ? 0F 16 F6 0F 12 36 0F 57 C0 0F 28 CE 0F C6 C8 ? 0F 28 C6 0F C6 C1 ? 0F 59 C0 0F 28 C8 0F C6 C8 ? 0F 58 C8 0F 28 C1 0F C6 C1 ? 0F 58 C1 F3 0F 51 C0 F3 0F 59 05 ? ? ? ? F3 0F 5A C0 E8 ? ? ? ? 0F 28 C8 F2 0F C2 C8 07 66 0F 54 C8 BA ? ? ? ? F2 0F 2C C9 66 0F 2E 05 ? ? ? ? 0F 42 D1 8B F2 0F 57 C0 F3 0F 2A C6 0F C6 C0 ? 0F 5E F0 85 F6 7E ? 8B EE 0F 29 74 24 ? 48 8D 54 24 ? 48 8B CB E8 ? ? ? ? FF CD 75 ? 85 F6 0F 85");

    //CREATE_HOOK(DrawScene);
    


    MH_EnableHook(MH_ALL_HOOKS);
    Util::log("All Hooks created successfully\n");
    return true;
}

void Hooks::UnhookAll() {
    shWglSwapBuffers.reset();
    shDoMoveCycle.reset();
    shHandleScreenShotting.reset();
    shOnUserInput.reset();
    shSetCursorHidden.reset();
	shOnChat.reset();
}
