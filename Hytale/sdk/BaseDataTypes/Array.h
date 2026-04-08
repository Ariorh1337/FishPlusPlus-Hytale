/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

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
};