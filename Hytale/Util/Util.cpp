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

bool Util::IsValidPtr(uint64_t ptr) {
	return IsValidPtr((void*)ptr);
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

uint64_t Util::RelativeVirtualAddress(uint64_t address, int opcode_size, int opcode_length) {
	if (!address)
		return 0;

	return (uint64_t) (*(int*) (address + opcode_size) + address + opcode_length);
}

uint64_t Util::PatternScan(const char* signature, const char* module) {
	const auto module_handle = GetModuleHandleA(module); // instead of using the string to find the handle, we're a dll and injected into the process already we can simplify it by using nullptr instead

	static auto pattern_to_byte = [](const char* pattern) {
		auto bytes = std::vector<int>{ };
		auto start = const_cast<char*>(pattern);
		auto end = const_cast<char*>(pattern) + std::strlen(pattern);

		for (auto current = start; current < end; ++current) {
			if (*current == '?') {
				++current;

				if (*current == '?')
					++current;

				bytes.push_back(-1);
			} else {
				bytes.push_back(std::strtoul(current, &current, 16));
			}
		}
		return bytes;
	};

	auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_handle);
	auto nt_headers =
		reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uint8_t*>(module_handle) + dos_header->e_lfanew);

	auto size_of_image = nt_headers->OptionalHeader.SizeOfImage;
	auto pattern_bytes = pattern_to_byte(signature);
	auto scan_bytes = reinterpret_cast<std::uint8_t*>(module_handle);

	auto s = pattern_bytes.size();
	auto d = pattern_bytes.data();

	for (auto i = 0ul; i < size_of_image - s; ++i) {
		bool found = true;

		for (auto j = 0ul; j < s; ++j) {
			if (scan_bytes[i + j] != d[j] && d[j] != -1) {
				found = false;
				break;
			}
		}
		if (found)
			return (uint64_t) &scan_bytes[i];
	}

	return 0;
}