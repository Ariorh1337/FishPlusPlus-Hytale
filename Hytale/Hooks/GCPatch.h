#pragma once
/**
 * GCPatch -- NativeAOT GC Stack-Walker patch infrastructure.
 *
 * Patches the .NET NativeAOT garbage collector to safely handle stack frames
 * from our injected DLL and MinHook trampolines. Without these patches,
 * the GC calls RaiseFailFastException when it encounters unknown PCs.
 *
 * Also registers UNWIND_INFO tables for all MinHook trampolines so that
 * RtlLookupFunctionEntry / RtlVirtualUnwind work correctly.
 */

#include <vector>
#include <utility>

 /**
  * Patch 1: Redirects the GC FailFast path to a code cave that installs
  * a fake ICodeManager (no-op EnumGcRefs + manual UnwindStackFrame).
  * Must be called BEFORE creating any MinHook hooks.
  */
bool PatchGCStackWalker();

/**
 * Patch 2: Wraps fn_KeepUnwinding's vtable[5] call with SafeUnwind1
 * to catch AVs from stale method_info on trampoline frames.
 * Must be called BEFORE creating any MinHook hooks.
 */
bool PatchGCStackWalkerKeepUnwinding();

/**
 * Registers RUNTIME_FUNCTION / UNWIND_INFO tables for all MinHook
 * trampoline pages so that RtlLookupFunctionEntry can find them.
 * Call after MH_EnableHook(MH_ALL_HOOKS).
 *
 * @param hooks  Pairs of (trampoline_address, original_function_address)
 */
void RegisterAllTrampolinePages(const std::vector<std::pair<void*, void*>>& hooks);
