/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include <stdint.h>

template<typename T>
struct Array {
	char pad_0[0x8];    // 0x00
	int count;          // 0x08
	char pad_C[0x4];    // 0x0C
	T list[1];          // 0x10

    T get(int index) {
        if (index < 0 || index >= count)
			return T();
        return list[index];
    }

    T getUnsafe(int index) {
        return list[index];
    }

    // Allocate a new array via the engine's allocation primitive
    static Array<T>* createArray(int length, void* methodTable) {
        if (!methodTable) return nullptr;
        // RhpNewArray signature: void*(void* mt, int length)
        using RhpNewArrayFn = void*(*)(void*, int);
        
        uint64_t addr = 0; // We must provide it
        // We can't easily include SigManager.h here due to cyclic includes sometimes,
        // so we'll just require the caller to pass it or we use extern.
        extern uint64_t g_RhpNewArrayAddress;
        if (!g_RhpNewArrayAddress) return nullptr;
        
        RhpNewArrayFn RhpNewArray = reinterpret_cast<RhpNewArrayFn>(g_RhpNewArrayAddress);
        return (Array<T>*)RhpNewArray(methodTable, length);
	}
};