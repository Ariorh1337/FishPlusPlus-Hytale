#pragma once
#include <cstdint>

template<typename Key, typename Value>
struct DictionaryPair {
	char pad_0x00[0x8];					// 0x00
	uint64_t value;						// 0x08
	DictionaryPair<Key, Value>* next;	// 0x10
	uint64_t key;						// 0x18

	Key getKey() {
		return *(Key*)((uint64_t)this + 0x18);
	}

	Value getValue() {
		return *(Value*)((uint64_t)this + 0x8);
	}
};


template<typename Key, typename Value>
struct ConcurrentDictionary {
	char pad_0x00[0x8];			// 0x00
	void* empty;				// 0x08
	Array<DictionaryPair<Key, Value>*>* bucket; // 0x10
	void* field_0x18;			// 0x18
	void* field_0x20;			// 0x20

	bool TryGetValue(Key objectKey, Value* outValue) {
		for (int i = 0; i < bucket->count; i++) {
			DictionaryPair<Key, Value>* pair = bucket->getUnsafe(i);
			while (pair && (uint64_t)pair > 0x10000) {
				if (pair->getKey() == objectKey) {
					*outValue = pair->getValue();
					return true;
				}
				pair = pair->next;
			}
		}
		return false;
	}
};