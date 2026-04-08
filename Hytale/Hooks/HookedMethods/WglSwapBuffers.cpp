/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"
#include "Features/FeatureHandler.h"
#include "Features/ConfigHandler.h"
#include "Renderer/FrameBufferRenderer/FrameBuffers.h"


static void* GetAnyGLFuncAddress(const char* name) {
    void* p = (void*) wglGetProcAddress(name);
    if (p == nullptr || p == (void*) 0x1 || p == (void*) 0x2 ||
        p == (void*) 0x3 || p == (void*) -1) {
        static HMODULE module = GetModuleHandleA("opengl32.dll");
        if (!module)
            module = LoadLibraryA("opengl32.dll");

        p = (void*) GetProcAddress(module, name);
    }
    return p;
}

#pragma optimize("", off)
#pragma runtime_checks("", off)
__declspec(safebuffers) __declspec(noinline)
BOOL WINAPI Hooks::hkWglSwapBuffers(HDC hdc) {
    if (!initialized) {
        if (!Util::IsValidPtr(Util::app))
            return Hooks::oWglSwapBuffers(hdc);

        if (!gladLoadGLLoader((GLADloadproc) GetAnyGLFuncAddress)) {
			Util::log("Failed to initialize GLAD\n");
            return -1;
        }
        Shaders::initShaders();
        Fonts::initFonts();

        Renderer2D::InitRenderer();

        FrameBuffers::initFBOS();

        menu = std::make_unique<Menu>(hdc);

        FeatureHandler::Init();

        
        if (!ConfigHandler::FishDirectoryExists()) {
            ConfigHandler::CreateFishDirectory();
            ConfigHandler::SaveConfig("immediateConfig", false);
        }
        else {
            ConfigHandler::LoadConfig("immediateConfig", false);
        }
        

        initialized = true;
    }

    //Save the config every 2 minutes
    if ((int)Util::GetTime() % 120 == 0)
        ConfigHandler::SaveConfig("immediateConfig", false);

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

        if (((oldWindowWidth != Util::app->Engine->Window->WindowWidth) || (oldWindowHeight != Util::app->Engine->Window->WindowHeight))) {
            FrameBuffers::resizeAll(Util::app->Engine->Window->WindowWidth, Util::app->Engine->Window->WindowHeight);
        }
            
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
            return Hooks::oWglSwapBuffers(hdc);
        }

        Fonts::Figtree->RenderText(std::format("App: 0x{:x}", reinterpret_cast<uintptr_t>(Util::app)), 0.0f, 10.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("AppInGame: 0x{:x}", reinterpret_cast<uintptr_t>(Util::app->appInGame)), 0.0f, 20.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("GameInstance: 0x{:x}", reinterpret_cast<uintptr_t>(Util::getGameInstance())), 0.0f, 30.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("LocalPlayer: 0x{:x}", reinterpret_cast<uintptr_t>(Util::getLocalPlayer())), 0.0f, 40.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("DMC: 0x{:x}", reinterpret_cast<uintptr_t>(Util::GetMovementController())), 0.0f, 50.0f, 0.5f, Color::White());
        Fonts::Figtree->RenderText(std::format("OptionsHelper: 0x{:x}", reinterpret_cast<uintptr_t>(Globals::optionsHelper)), 0.0f, 60.0f, 0.5f, Color::White());

        Fonts::Figtree->RenderText(std::format("Fish++ Hytale by LimitlessChicken aka milaq", reinterpret_cast<uintptr_t>(Util::app)), 500.0f, 10.0f, 0.5f, Color::White());

        menu->Run(deltaTime);
    }

    InputSystem::inputMutex.lock();
    InputSystem::keysPressed.clear();
    InputSystem::keysDepressed.clear();
    InputSystem::inputMutex.unlock();


    return Hooks::oWglSwapBuffers(hdc);
}
#pragma runtime_checks("", restore)
#pragma optimize("", on)