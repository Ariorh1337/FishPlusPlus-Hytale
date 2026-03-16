/*
 * Copyright (c) FishPlusPlus.
 */
#include "Core.h"
#include "Hooks.h"
#include "external/safetyhook/safetyhook.hpp"
#include "Events/EventRegister.h"
#include "Features/FeatureHandler.h"

typedef int SDL_bool;
#define SDL_TRUE  1
#define SDL_FALSE 0

static bool initialized = false;
static bool initialized3D = false;

inline SafetyHookInline shWglSwapBuffers{ };
inline SafetyHookInline shDoMoveCycle{ };
inline SafetyHookInline shHandleScreenShotting{ };
inline SafetyHookInline shOnUserInput{ };
inline SafetyHookInline shSetCursorHidden{ };
inline SafetyHookInline shOnChat{ };

static std::unique_ptr<Menu> menu;

#define CREATE_HOOK(name, pattern) \
uint64_t name##Address = PatternScan(pattern);\
if (MH_CreateHook((LPVOID)name##Address, &H##name, reinterpret_cast<LPVOID*>(&o##name)) != MH_OK) {\
    std::cout << "failed to create hook: " << #name << "\n";\
    return false;\
}\

#define safety_hook_method(name)                                                            \
        sh##name = safetyhook::create_inline(SM::name##Address, &H##name);                  \
        if (!sh##name) {                                                                    \
            Util::log("Failed to hook %s\n", #name);                                        \
            return false;                                                                   \
        }                                                                                   \
        o##name = sh##name.original<name>();                                                \
        Util::log("Hooked %s: 0x%llX\n", #name, SM::name##Address);                                


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

        //Renderer3D renderer3D;
        //Render3DEvent render3DEvent(renderer3D);
        //FeatureDispatcher::DispatchEvent(render3DEvent);
        //renderer3D.Render();

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

uint64_t __fastcall HDoMoveCycle(DefaultMovementController* dmc, Vector3* offset) {

	//EventRegister::DoMoveCycleEvent.Invoke(dmc, *offset);

        /*
        MoveCycleEvent event(*dmc, *offset);
        FeatureDispatcher::DispatchEvent(event);
        */
        /*
        if (Util::ShouldInteractWithGame()) {
            //Util::log("DoMoveCycle called with DMC: 0x%llX, offset: (%f, %f, %f)\n", dmc, offset->x, offset->y, offset->z);
            Vector3 dir = *offset;
            dmc->SpeedMultiplier = 1.0f;
            float yawRad = Util::getLocalPlayer()->yawRad;
            float forwardX = -sin(yawRad);
            float forwardZ = -cos(yawRad);

            float strafeX = forwardZ;
            float strafeZ = -forwardX;

            dmc->Velocity.x = 0.0f;
            dmc->Velocity.z = 0.0f;
            dir.x = 0.0f;
            dir.z = 0.0f;
			float currentSpeed = FeatureDispatcher::GetFeatureFloatValue("Speed");

            if (InputSystem::IsKeyHeld(SDL_SCANCODE_W))
                dir += Vector3(forwardX * currentSpeed, dir.y, forwardZ * currentSpeed);
            if (InputSystem::IsKeyHeld(SDL_SCANCODE_S))
                dir += Vector3(-forwardX * currentSpeed, dir.y, -forwardZ * currentSpeed);
            if (InputSystem::IsKeyHeld(SDL_SCANCODE_A))
                dir += Vector3(strafeX * currentSpeed, dir.y, strafeZ * currentSpeed);
            if (InputSystem::IsKeyHeld(SDL_SCANCODE_D))
                dir += Vector3(-strafeX * currentSpeed, dir.y, -strafeZ * currentSpeed);
			offset = &dir;
        }*/
    return Hooks::oDoMoveCycle(dmc, offset);
}

uint64_t __fastcall HHandleScreenShotting(App* app) {
    if (Util::app != app)
        Util::app = app;

    SDK::Main();

    return Hooks::oHandleScreenShotting(app);
}


uint64_t* __fastcall HOnUserInput(uint64_t thisptr, int* a2) {
    SDL_Scancode key = (SDL_Scancode) (a2[6]);
    if (*a2 == 768) {
        if (*((bool*) a2 + 37))
            return Hooks::oOnUserInput(thisptr, a2);

        InputSystem::keysPressed.insert(key);
        InputSystem::keysHeld.insert(key);
        InputSystem::keysUnheld.erase(key);
    }

    if (*(int*) a2 == 769) {
        InputSystem::keysHeld.erase(key);
        InputSystem::keysUnheld.insert(key);
        InputSystem::keysDepressed.insert(key);
    }

    return Hooks::oOnUserInput(thisptr, a2);
}

void __fastcall HSetCursorHidden(Window* window, bool hidden) {
    if (Menu::isMenuOpen())
        hidden = false;
       
    Hooks::oSetCursorHidden(window, hidden);
}
/*
void __fastcall HWeatherUpdate(uint64_t thisptr, float deltaTime) {

    Hooks::oWeatherUpdate(thisptr, deltaTime);

    float* vec3 = (float*)((uintptr_t)thisptr + 0x364);

    //vec3[0] = 2.0f;

    //vec32[0] = 2.0f;

}

void __fastcall HSetActiveHotbarSlot(uint64_t thisptr, unsigned int slot, bool triggerInteraction) {
    Hooks::oSetActiveHotbarSlot(thisptr, slot, triggerInteraction);
}
*/

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
    Hooks::oOnChat(thisptr, a2);
}

uint64_t __fastcall HDrawScene(uint64_t thisptr) {
    return Hooks::oDrawScene(thisptr);
    try {
        //Renderer3D renderer3D;
        //Render3DEvent render3DEvent(renderer3D);
        //FeatureDispatcher::DispatchEvent(render3DEvent);
        //renderer3D.Render();
    }
    catch (...) {
        std::cout << "Exception in DrawScene\n";
    }
}

bool Hooks::CreateHooks() {
    Util::log("Creating MH Hooks\n");
    if (MH_Initialize() != MH_OK) {
		Util::log("Failed to initialize MinHook");
        return false;
    }

    void* pWglSwapBuffers = GetProcAddress(GetModuleHandleA("opengl32.dll"), "wglSwapBuffers");
    if (MH_CreateHook(pWglSwapBuffers, &HWglSwapBuffers, reinterpret_cast<LPVOID*>(&oWglSwapBuffers)) != MH_OK) {
        std::cout << "Failed to create hook HWglSwapBuffers";
        return false;
    }

    CREATE_HOOK(DoMoveCycle, "55 41 57 41 56 41 55 41 54 57 56 53 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 44 0F 29 84 24 ? ? ? ? 48 8D AC 24 ? ? ? ? 33 C0 48 89 85 ? ? ? ? 0F 57 E4 48 B8");
    CREATE_HOOK(HandleScreenShotting, "55 41 57 41 56 41 55 41 54 57 56 53 48 81 EC ? ? ? ? 48 8D AC 24 ? ? ? ? 33 C0 48 89 45 ? 0F 57 E4 0F 29 65 ? 48 89 45 ? 48 8B D9 48 8B 4B ? 48 8B 49");
    CREATE_HOOK(OnUserInput, "41 57 41 56 41 55 41 54 57 56 55 53 48 83 EC ? 33 C0 48 89 44 24 ? 0F 57 E4 0F 29 64 24 ? 0F 29 64 24 ? 0F 29 64 24 ? 48 89 44 24 ? 48 8B D9 48 8B F2 8B 3E");
    CREATE_HOOK(SetCursorHidden, "55 57 56 53 48 83 EC ? 48 8D 6C 24 ? 33 C0 48 89 45 ? 48 89 45 ? 48 8B D9 8B F2");

    //CREATE_HOOK(UpdateInputStates, "57 56 53 48 83 EC ? 48 8B D9 8B F2 48 8B 4B ? 48 85 C9 0F 84");
    //CREATE_HOOK(SetActiveHotbarSlot, "55 41 56 57 56 53 48 83 EC ? 48 8D 6C 24 ? 48 8B D9 8B F2 48 83 7B");
    //CREATE_HOOK(WeatherUpdate, "57 56 55 53 48 83 EC ? 0F 29 74 24 ? 48 8B D9 48 8B F2 48 8B 4B ? 48 8B 89 ? ? ? ? 48 8B 79 ? 80 BB ? ? ? ? ? 74 ? 80 7B ? ? 0F 85 ? ? ? ? 48 8B CF 4C 8D 1D ? ? ? ? 41 FF 13 85 C0 0F 85 ? ? ? ? 0F B6 83 ? ? ? ? 88 83 ? ? ? ? F3 0F 10 76 ? 0F 16 F6 0F 12 36 0F 57 C0 0F 28 CE 0F C6 C8 ? 0F 28 C6 0F C6 C1 ? 0F 59 C0 0F 28 C8 0F C6 C8 ? 0F 58 C8 0F 28 C1 0F C6 C1 ? 0F 58 C1 F3 0F 51 C0 F3 0F 59 05 ? ? ? ? F3 0F 5A C0 E8 ? ? ? ? 0F 28 C8 F2 0F C2 C8 07 66 0F 54 C8 BA ? ? ? ? F2 0F 2C C9 66 0F 2E 05 ? ? ? ? 0F 42 D1 8B F2 0F 57 C0 F3 0F 2A C6 0F C6 C0 ? 0F 5E F0 85 F6 7E ? 8B EE 0F 29 74 24 ? 48 8D 54 24 ? 48 8B CB E8 ? ? ? ? FF CD 75 ? 85 F6 0F 85");

    //CREATE_HOOK(OnChat, "56 53 48 83 EC ? 48 8B F1 48 8B DA 38 1B 48 8B CB");
    //CREATE_HOOK(DrawScene, "57 56 55 53 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 44 0F 29 84 24 ? ? ? ? 33 C0 48 89 84 24 ? ? ? ? 48 8B D9");
    


    MH_EnableHook(MH_ALL_HOOKS);
    Util::log("All MinHooks created successfully\n");
    return true;
}

bool Hooks::CreateSafetyHooks() {
    Util::log("Creating SafetyHook Hooks\n");

    safety_hook_method(WglSwapBuffers);
    safety_hook_method(DoMoveCycle);
    safety_hook_method(HandleScreenShotting);
    safety_hook_method(OnUserInput);
    safety_hook_method(SetCursorHidden);
    safety_hook_method(OnChat);

    Util::log("All SafetyHook hooks created successfully\n");
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
