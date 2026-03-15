/*
 * Copyright (c) FishPlusPlus.
 */
#include "core.h"
#include "Util.h"

#include "../sdk/Entity.h"

void Util::allocate_console() {
	if (console_allocated)
		return;

	AllocConsole();
	freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);

	console_allocated = true;
}

void Util::free_console() {
	if (!console_allocated)
		return;

	fclose(reinterpret_cast<FILE*>(stdout));
	FreeConsole();

	console_allocated = false;
}

void Util::log(const char* fmt, ...) {
	if (!console_allocated)
		return;

	std::time_t now = std::time(nullptr);
	std::tm timeinfo;
	localtime_s(&timeinfo, &now);

	char buffer[80];
	strftime(buffer, sizeof(buffer), "[%H:%M:%S]", &timeinfo);

	auto console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console_handle, 11);
	printf(buffer);
	SetConsoleTextAttribute(console_handle, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
	printf(" ");

	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

bool Util::IsValidPtr(void* ptr) {
	if (!ptr)
		return false;

	if (ptr < (void*) 0x10000000)
		return false;

	if (ptr > (void*) 0x7FFFFFFFFFFF)
		return false;

	return true;
}

GameInstance* Util::getGameInstance() {
	ValidPtr(app);
	ValidPtr(app->appInGame);
	return app->appInGame->gameInstance;
}

Entity* Util::getLocalPlayer() {
	GameInstance* gameInstance = getGameInstance();
	ValidPtr(gameInstance);
	return gameInstance->Player;
}

DefaultMovementController* Util::GetMovementController() {
	GameInstance* gameInstance = getGameInstance();
	ValidPtr(gameInstance);
	CharacterControllerModule* module = gameInstance->CharacterControllerModule;
	ValidPtr(module);
	return module->MovementController;
}

Camera* Util::getCamera() {
	GameInstance* gameInstance = getGameInstance();
	ValidPtr(gameInstance);

	StructBeforeCam* structBeforeCam = gameInstance->Camera;
	ValidPtr(structBeforeCam);

	return structBeforeCam->Camera;
}

CameraModule* Util::getCameraModule() {
	GameInstance* gameInstance = getGameInstance();
	ValidPtr(gameInstance);

	return gameInstance->CameraModule;
}

double Util::GetTime() {
	static LARGE_INTEGER freq;
	static bool initialized = false;
	if (!initialized) {
		QueryPerformanceFrequency(&freq);
		initialized = true;
	}

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return (double)now.QuadPart / (double)freq.QuadPart;
}