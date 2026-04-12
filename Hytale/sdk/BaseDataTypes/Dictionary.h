#pragma once
#include <cstdint>
#include <vector>

template<typename Key, typename Value>
struct Entry {
	Key key;        // +0
	Value value;    // +8  ← RETURNED
	int hashCode;   // +16
	int next;       // +20
};

template<typename Key, typename Value>
struct Dictionary {
	void* klass;                         // +0x00
	void* monitor;                       // +0x08

	Array<int>* buckets;                 // +0x10  (a1[1])
	Array<Entry<Key, Value>*>* entries;   // +0x18  (a1[2])
	void* comparer;                      // +0x20  (a1[3])
	void* keys;                          // +0x28
	void* values;                        // +0x30

	char pad_38[0x18];                   // +0x38

	uint64_t fast_mod_multiplier;        // +0x50  (a1[6])

	int count;                           // +0x58
	int free_list;                       // +0x5C
	int free_count;                      // +0x60
	int version;                         // +0x64

	uint32_t FastMod(uint32_t hash, uint32_t size, uint64_t multiplier) {
		return (uint32_t) ((size * (((uint64_t) multiplier * hash) >> 32) + 1) >> 32);
	}

	int GetHash(void* comparer, Key key) {
		using GetHashCodeFn = int(*)(void*, Key);
		auto fn = *(GetHashCodeFn*) ((*(uint64_t*) comparer) + 120);
		return fn(comparer, key);
	}

	bool Equals(void* comparer, Key a, Key b) {
		using EqualsFn = bool(*)(void*, Key, Key);
		auto fn = *(EqualsFn*) ((*(uint64_t*) comparer) + 112);
		return fn(comparer, a, b);
	}


	Entry<Key, Value>* FindEntry(Key key) {
		if (!key)
			return nullptr;

		if (!buckets)
			return nullptr;

		int hash = GetHash(comparer, key);

		uint32_t bucket = FastMod(
			hash,
			buckets->count,
			fast_mod_multiplier
		);

		int index = buckets->getUnsafe(bucket) - 1;
		int loops = 0;

		while (index >= 0) {
			if (index >= entries->count)
				return nullptr;

			Entry<Key, Value>* entry = entries->getUnsafe(index);

			if (entry->hashCode == hash &&
				Equals(comparer, entry->key, key)) {
				return entry;
			}

			index = entry->next;

			if (++loops > entries->count)
				return nullptr; // corruption guard
		}

		return nullptr;
	}

	Value* GetValue( Key key) {
		auto entry = FindEntry(key);
		if (!entry)
			return nullptr;

		return &entry->value;
	}

	std::vector<Key> GetKeys() {
		std::vector<Key> keysVector;
		for (int i = 0; i < entries->count; i++) {
			Entry<Key, Value>* entry = entries->getUnsafe(i);
			if (entry->hashCode >= 0) { // valid entry
				keysVector.push_back(entry->key);
			}
		}
		return keysVector;
	}
};