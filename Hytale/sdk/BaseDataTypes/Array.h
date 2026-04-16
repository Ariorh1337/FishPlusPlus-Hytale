/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once
#include <stdint.h>
#include "Util/SigManager.h"

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

    // Allocate a new managed array via RhpNewArray.
    // methodTable must be a valid NativeAOT MethodTable* for the desired array type.
    // Returns nullptr if SM::RhpNewArray_GenericAddress is not yet resolved.
    static Array<T>* createArray(int length, void* methodTable) {
        if (!methodTable || !SM::RhpNewArray_GenericAddress) return nullptr;
        using RhpNewArrayFn = void*(*)(void*, int);
        return (Array<T>*)reinterpret_cast<RhpNewArrayFn>(SM::RhpNewArray_GenericAddress)(methodTable, length);
    }
};