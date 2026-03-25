/*
 * Copyright (c) FishPlusPlus.
 */
#include "Core.h"
#include "Hooks.h"
#include "Events/EventRegister.h"
#include "Features/FeatureHandler.h"
#include "Features/ActualFeatures/WorldModulate.h"

static bool initialized = false;
static bool initialized3D = false;

static int oldWindowWidth = 0;
static int oldWindowHeight = 0;

static std::unique_ptr<Menu> menu;
static std::unique_ptr<FramebufferRenderer> fboRenderer;

#define CREATE_HOOK(name) \
if (MH_CreateHook((LPVOID)SM::name##Address, &H##name, reinterpret_cast<LPVOID*>(&o##name)) != MH_OK) {\
    Util::log("Failed to hook %s\n", #name);\
    return false;\
}\

#define CREATE_SIG_HOOK(name, pattern) \
std::uintptr_t name##Address = Util::PatternScan(pattern);\
Util::log("Found %s sig at: 0x%llX - 0x%llX = 0x%lX\n", #name, name##Address, gameBase, (name##Address - gameBase));\
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

		fboRenderer = std::make_unique<FramebufferRenderer>(Shaders::postProcess.get());

        menu = std::make_unique<Menu>(hdc);

		FeatureHandler::Init();

        initialized = true;
    }

    static double lastTime = 0.0;
    double currentTime = Util::GetTime();
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    HWND hwnd = WindowFromDC(hdc);
    POINT current;
    GetCursorPos(&current);
    ScreenToClient(hwnd, &current);
    Util::cursorPosX = current.x;
    Util::cursorPosY = current.y;

    if (Util::IsValidPtr(Util::app)) {

        if (fboRenderer &&  ((oldWindowWidth != Util::app->Engine->Window->WindowWidth) || (oldWindowHeight != Util::app->Engine->Window->WindowHeight)))
			fboRenderer->resize(Util::app->Engine->Window->WindowWidth, Util::app->Engine->Window->WindowHeight);
        oldWindowWidth = Util::app->Engine->Window->WindowWidth;
		oldWindowHeight = Util::app->Engine->Window->WindowHeight;

        if (!Util::orthoProjMatInitialized) {
            Util::orthoProjMat = Matrix4x4::Orthographic(0.0f, Util::app->Engine->Window->WindowWidth, Util::app->Engine->Window->WindowHeight, 0.0f, -1.0f, 1.0f);
            Util::orthoProjMatInitialized = true;
        }
        if (!uninjecting)
            uninjecting = InputSystem::IsKeyPressed(SDL_SCANCODE_END);

        if (uninjecting) {
            MH_DisableHook(MH_ALL_HOOKS);
            Util::free_console();
            return oWglSwapBuffers(hdc);
		}

        Fonts::Figtree->RenderText(std::format("App: 0x{:x}", reinterpret_cast<uintptr_t>(Util::app)), 0.0f, 10.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("AppInGame: 0x{:x}", reinterpret_cast<uintptr_t>(Util::app->appInGame)), 0.0f, 20.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("GameInstance: 0x{:x}", reinterpret_cast<uintptr_t>(Util::getGameInstance())), 0.0f, 30.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("LocalPlayer: 0x{:x}", reinterpret_cast<uintptr_t>(Util::getLocalPlayer())), 0.0f, 40.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("DMC: 0x{:x}", reinterpret_cast<uintptr_t>(Util::GetMovementController())), 0.0f, 50.0f, 0.5f, Color::White());

        Fonts::Figtree->RenderText(std::format("Fish++ Hytale by LimitlessChicken aka milaq", reinterpret_cast<uintptr_t>(Util::app)), 500.0f, 10.0f, 0.5f, Color::White());

        menu->Run(deltaTime);
    }

    InputSystem::inputMutex.lock();
    InputSystem::keysPressed.clear();
    InputSystem::keysDepressed.clear();
    InputSystem::inputMutex.unlock();

    
    return oWglSwapBuffers(hdc);
}

void __fastcall HDoMoveCycle(DefaultMovementController* dmc, Vector3 offset) {
    if (!initialized)
        return;
    if (Util::IsValidPtr(dmc) && Util::IsValidPtr(Util::app) && Util::IsValidPtr(Util::app->appInGame) && Util::IsValidPtr(Util::getGameInstance()) && Util::IsValidPtr(Util::getLocalPlayer())) {
        EventRegister::DoMoveCycleEvent.Invoke(dmc, offset);
    }

    //EventRegister::DoMoveCycleEvent.Invoke(dmc, offset);
    Hooks::oDoMoveCycle(dmc, offset);
}

void __fastcall HHandleScreenShotting(App* app) {
    if (Util::app != app)
        Util::app = app;

    Hooks::oHandleScreenShotting(app);
    SDK::Main();
}

void __fastcall HOnUserInput(uint64_t thisptr, SDL_Event a2) {
    if (a2.type != SDL_KEYDOWN && a2.type != SDL_KEYUP) {
        Hooks::oOnUserInput(thisptr, a2);
        return;
    }

    SDL_Scancode key{ a2.key.scancode };
    bool shouldCallOriginal = true;

    if (a2.type == SDL_KEYDOWN) {
        if (a2.key.repeat) {
            Hooks::oOnUserInput(thisptr, a2);
            return;
        }

        InputSystem::inputMutex.lock();
        InputSystem::keysPressed.insert(key);
        InputSystem::keysHeld.insert(key);
        InputSystem::keysUnheld.erase(key);
        InputSystem::inputMutex.unlock();
    } else if (a2.type == SDL_KEYUP) {
        InputSystem::inputMutex.lock();
        InputSystem::keysHeld.erase(key);
        InputSystem::keysUnheld.insert(key);
        InputSystem::keysDepressed.insert(key);
        InputSystem::inputMutex.unlock();
    }

    Hooks::oOnUserInput(thisptr, a2);
}

void __fastcall HOnChat(uint64_t thisptr, uint64_t a2) {
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

void HDrawScene(GameInstance* thisptr) {
    Hooks::oDrawScene(thisptr);
    if (!initialized)
        return;

    Renderer3D renderer3D;
    EventRegister::Render3DEvent.Invoke(renderer3D);
    renderer3D.Render();

    fboRenderer->draw();
}

void HWeatherUpdate(uintptr_t a1, float deltaTime) {
    Hooks::oWeatherUpdate(a1, deltaTime);

    WorldModulate* worldModulate = static_cast<WorldModulate*>(FeatureHandler::GetFeatureFromName("WorldModulate"));
    if (!worldModulate)
        return;

    if (!worldModulate->IsActive())
        return;

    bool noFog = static_cast<ToggleSetting*>(worldModulate->GetSettingFromName("No Fog"))->GetValue();

    RecursiveSetting* fogChanger = static_cast<RecursiveSetting*>(worldModulate->GetSettingFromName("Fog Changer"));

    //TODO: Add the actual WeatherModule struct
    if (fogChanger->GetValue()) {
        float fogStart = static_cast<SliderSetting*>(fogChanger->GetSettingFromName("Start"))->GetValue();
        float fogEnd = static_cast<SliderSetting*>(fogChanger->GetSettingFromName("End"))->GetValue();
        Color color = static_cast<ColorSetting*>(fogChanger->GetSettingFromName("Color"))->GetValue();
        color = Color::Normalize(color);

        *(float*) ((uintptr_t) (a1 + 0x90)) = fogStart;
        *(float*) ((uintptr_t) (a1 + 0x94)) = fogEnd;
        *(float*) ((uintptr_t) (a1 + 0x110)) = color.r;
        *(float*) ((uintptr_t) (a1 + 0x114)) = color.g;
        *(float*) ((uintptr_t) (a1 + 0x118)) = color.b;
    }
    if (noFog)
        *(float*) ((uintptr_t) (a1 + 0x94)) = 0.0f;
}

void HDrawEntityCharactersAndItems(SceneRenderer* a1, bool useOcclusionCulling) {
    if (!Util::IsValidPtr(Util::app))
        return Hooks::oDrawEntityCharactersAndItems(a1, useOcclusionCulling);

    //Render entities through the walls
	glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, -1500000.0f);
    Hooks::oDrawEntityCharactersAndItems(a1, false);
    glPolygonOffset(1.0f, 1500000.0f);
    glDisable(GL_POLYGON_OFFSET_FILL);
   

    fboRenderer->bind();

    Hooks::oDrawEntityCharactersAndItems(a1, false);

    fboRenderer->unbind();
}

bool HFrameIterator_IsValid(GCInstance* instance) {
    if (!instance)
        return Hooks::oFrameIterator_IsValid(instance);

    bool isValid = Hooks::oFrameIterator_IsValid(instance);
    if (instance->address >= dllBase && instance->address <= dllBaseEnd) {
        Util::log("GC: Marking DLL frame 0x%llX with ActiveStackFrame\n", instance->address);
		instance->address = 0; // Clear address to prevent GC from trying to read method info
		instance->originalControlPC = 0;
        instance->codeManager = nullptr;
        instance->pendingFuncletFramePointer = nullptr;
        instance->framePointer = nullptr;
        instance->pConservativeStackRangeLowerBound = 0x0;
        instance->pConservativeStackRangeUpperBound = 0x0;
        instance->flags |= GCFlag::MethodStateCalculated;
        instance->flags |= GCFlag::SkipNativeFrames;
        return false;
    }

    return isValid;
}

bool Hooks::CreateHooks() {
    Util::log("Creating Hooks\n");

    if (MH_Initialize() != MH_OK) {
        Util::log("Failed to initialize MinHook");
        return false;
    }

    CREATE_SIG_HOOK(FrameIterator_IsValid, "48 83 79 ? ? 0F 95 C0 C3 CC CC CC CC CC CC CC 40 53"); // E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 49 8B 5D
    CREATE_HOOK(WglSwapBuffers);
    CREATE_HOOK(DoMoveCycle);
    CREATE_HOOK(HandleScreenShotting);
    CREATE_HOOK(OnUserInput);
    CREATE_HOOK(OnChat);
    CREATE_HOOK(DrawEntityCharactersAndItems);
    //CREATE_HOOK(SetCursorHidden, "55 57 56 53 48 83 EC ? 48 8D 6C 24 ? 33 C0 48 89 45 ? 48 89 45 ? 48 8B D9 8B F2");

    //CREATE_HOOK(UpdateInputStates, "57 56 53 48 83 EC ? 48 8B D9 8B F2 48 8B 4B ? 48 85 C9 0F 84");
    //CREATE_HOOK(SetActiveHotbarSlot, "55 41 56 57 56 53 48 83 EC ? 48 8D 6C 24 ? 48 8B D9 8B F2 48 83 7B");
    CREATE_SIG_HOOK(WeatherUpdate, "41 57 41 56 41 55 41 54 57 56 55 53 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 44 0F 29 84 24 ? ? ? ? 44 0F 29 8C 24 ? ? ? ? 0F 57 E4 0F 29 64 24 ? 0F 29 64 24 ? 48 B8"); //E8 ? ? ? ? 48 8B 4B ? 48 8B 49 ? BA ? ? ? ? 39 09 E8 ? ? ? ? 80 BB ? ? ? ? ? 75 ? 48 8B 8B ? ? ? ? F3 0F 10 8B ? ? ? ? 39 09 E8 ? ? ? ? 48 8B 8B

    CREATE_HOOK(DrawScene);

    MH_EnableHook(MH_ALL_HOOKS);

    Util::log("All Hooks created and registered successfully\n");
    return true;
}