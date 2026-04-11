/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include "core.h"
#include "Math/Vector3.h"

enum GCFlag : uint32_t {
    // If this flag is set, each unwind will apply a -1 to the ControlPC.  This is used by EH to ensure
    // that the ControlPC of a callsite stays within the containing try region.
    ApplyReturnAddressAdjustment = 1,

    // Used by the GC stackwalk, this flag will ensure that multiple funclet frames for a given method
    // activation will be given only one callback.  The one callback is given for the most nested physical
    // stack frame of a given activation of a method.  (i.e. the leafmost funclet)
    CollapseFunclets             = 2,

    // This is a state returned by Next() which indicates that we just crossed an ExInfo in our unwind.
    ExCollide                    = 4,

    // If a hardware fault frame is encountered, report its control PC at the binder-inserted GC safe
    // point immediately after the prolog of the most nested enclosing try-region's handler.
    RemapHardwareFaultsToSafePoint = 8,

    MethodStateCalculated = 0x10,

    // This is a state returned by Next() which indicates that we just unwound a reverse pinvoke method
    UnwoundReversePInvoke = 0x20,

    // The thread was interrupted in the current frame at the current IP by a signal, SuspendThread or similar.
    ActiveStackFrame = 0x40,

    // When encountering a reverse P/Invoke, unwind directly to the P/Invoke frame using the saved transition frame.
    SkipNativeFrames = 0x80,

    // Set SP to an address that is valid for funclet resumption (x86 only)
    UpdateResumeSp = 0x100,

    GcStackWalkFlags = (CollapseFunclets | RemapHardwareFaultsToSafePoint | SkipNativeFrames),
    EHStackWalkFlags = (ApplyReturnAddressAdjustment | UpdateResumeSp),
    StackTraceStackWalkFlags = GcStackWalkFlags
};

struct GCData {
	char pad[0x18];            // 0x0
	uint64_t object;           // 0x18
	uint64_t regionStart;      // 0x20
    uint32_t regionEnd;        // 0x28
};

// 128-bit floating point type for XMM registers (16 bytes)
struct Fp128 {
    uint64_t Low;
    uint64_t High;
};

struct REGDISPLAY {
    uintptr_t* pRax;
    uintptr_t* pRcx;
    uintptr_t* pRdx;
    uintptr_t* pRbx;
    uintptr_t* pRbp;
    uintptr_t* pRsi;
    uintptr_t* pRdi;
    uintptr_t* pR8;
    uintptr_t* pR9;
    uintptr_t* pR10;
    uintptr_t* pR11;
    uintptr_t* pR12;
    uintptr_t* pR13;
    uintptr_t* pR14;
    uintptr_t* pR15;
    uintptr_t  SP; // Stack pointer (RSP)
    uintptr_t  IP; // Instruction pointer (RIP)
    uintptr_t  SSP; // Shadow Stack Pointer (for Control Flow Guard)
    Fp128      Xmm[16 - 6]; // XMM6-XMM15 (128-bit each, used for floating-point and SIMD operations)
};

struct MethodInfo {
    char* AssemblyName;
    uint32_t MethodToken;
    uint32_t HotRVA;
    uint32_t HotLength;
    char* Name;
    uint32_t ColdRVA;
    uint32_t ColdLength;
};

struct PreservedRegPtrs {
    uintptr_t* pRbp;
    uintptr_t* pRdi;
    uintptr_t* pRsi;
    uintptr_t* pRbx;
    uintptr_t* pR12;
    uintptr_t* pR13;
    uintptr_t* pR14;
    uintptr_t* pR15;
};

struct GCInstance {
    void* thread;                      // 0x0 or 0
    GCData* gcData;                    // 0x8 or 8
    void* framePointer;                // 0x10 or 16
    uint64_t address;                  // 0x18 or 24 - (m_ControlPC ) Current RIP being scanned 
	REGDISPLAY RegisterSet;            // 0x20 or 32 - Register state for current frame
    void* codeManager;                 // 0x150 or 336 - Stack walker cache/unwinder
	MethodInfo methodInfo;             // 0x158 or 344 - Method information for current frame
	char pad160[0x20];                 // 0x160 or 352 (32 bytes padding)
	void* effectiveSafePointAddres;    // 0x1A0 or 416 - (address Copy) The actual safe point address to report for this frame, which may differ from the ControlPC if EH has requested return address adjustment or remapping of hardware faults to safe points
    uint64_t pConservativeStackRangeLowerBound; // 0x1A8 or 424 - Copy of crawl state
    uint64_t pConservativeStackRangeUpperBound; // 0x1B0 or 432 - Current frame crawl state
    int flags;                         // 0x1B8 or 440 - GC flags (see GCFlag enum)
    char pad1BC[0x4];                  // 0x1BC or 444
    void* pNextExInfo;                 // 0x1C0 or 448 - RBP value Frame/exception chain
	void* pendingFuncletFramePointer;  // 0x1C8 or 456 - RBP value for pending funclet (used by EH to unwind past funclets)
	PreservedRegPtrs funcletPtrs;      // 0x1D0 or 464 - Storage for preserved registers when unwinding funclets
	uint64_t originalControlPC;        // 0x210 or 528 - Original ControlPC before any funclet unwinding (used by EH to ensure correct IP reporting)
	uint64_t pPreviousTransitionFrame; // 0x218 or 536 - Used by reverse P/Invoke to store the previous transition frame for proper unwinding back to native frames
};

namespace HookData {
    inline bool initialized = false;
    inline bool initialized3D = false;

    inline int oldWindowWidth = 0;
    inline int oldWindowHeight = 0;

    inline std::unique_ptr<Menu> menu;
    inline std::unique_ptr<FramebufferRenderer> fboRenderer;

	inline bool queueTeleport = false;
	inline Vector3 teleportTarget = Vector3(0, 0, 0);

    
    inline std::vector<Vector3> goodtestBlock;
}

using namespace HookData;

namespace Hooks {
    typedef BOOL(WINAPI* WglSwapBuffers)(HDC hdc);
    inline WglSwapBuffers oWglSwapBuffers = nullptr;
    extern BOOL WINAPI hkWglSwapBuffers(HDC hdc);

	typedef void(__fastcall* DoMoveCycle)(DefaultMovementController* dmc, Vector3 offset);
	inline DoMoveCycle oDoMoveCycle = nullptr;
	extern void __fastcall hkDoMoveCycle(DefaultMovementController* dmc, Vector3 offset);

	typedef void(__fastcall* HandleScreenShotting)(App* app);
	inline HandleScreenShotting oHandleScreenShotting = nullptr;
	extern void __fastcall hkHandleScreenShotting(App* app);

	typedef void(__fastcall* OnUserInput)(uint64_t instance, SDL_Event event);
	inline OnUserInput oOnUserInput = nullptr;
	extern void __fastcall hkOnUserInput(uint64_t instance, SDL_Event event);

	typedef void(__fastcall* WeatherUpdate)(uint64_t instance, float deltaTime);
	inline WeatherUpdate oWeatherUpdate = nullptr;
	extern void __fastcall hkWeatherUpdate(uint64_t instance, float deltaTime);

	typedef void(__fastcall* OnChat)(uint64_t instance, HytaleString* chatString);
	inline OnChat oOnChat = nullptr;
	extern void __fastcall hkOnChat(uint64_t instance, HytaleString* chatString);

	typedef void(__fastcall* DrawPostEffect)(GameInstance* instance);
	inline DrawPostEffect oDrawPostEffect = nullptr;
	extern void __fastcall hkDrawPostEffect(GameInstance* instance);

    typedef void(__fastcall* DrawEntityCharactersAndItems)(SceneRenderer* instance, bool useOcclusionCulling);
	inline DrawEntityCharactersAndItems oDrawEntityCharactersAndItems = nullptr;
	extern void __fastcall hkDrawEntityCharactersAndItems(SceneRenderer* instance, bool useOcclusionCulling);

	typedef void(__fastcall* BuildGeometry)(void* instance, ChunkColumn* a2, int chunkX, int chunkY, int chunkZ, int64_t a6, int64_t a7, int64_t a8, int64_t a9, int64_t a10, int64_t a11, int64_t a12, int a13, int a14, int64_t* a15);
	inline BuildGeometry oBuildGeometry = nullptr;
	extern void __fastcall hkBuildGeometry(void* instance, ChunkColumn* a2, int chunkX, int chunkY, int chunkZ, int64_t a6, int64_t a7, int64_t a8, int64_t a9, int64_t a10, int64_t a11, int64_t a12, int a13, int a14, int64_t* a15);


    typedef void(__fastcall* SetClientBlock)(void* instance, int x, int y, int z, uint32_t newID, int arg6, int arg7, bool notify);
    inline SetClientBlock oSetClientBlock = nullptr;
    extern void __fastcall hkSetClientBlock(void* instance, int x, int y, int z, uint32_t newID, int arg6, int arg7, bool notify);

	bool CreateHooks();
};    