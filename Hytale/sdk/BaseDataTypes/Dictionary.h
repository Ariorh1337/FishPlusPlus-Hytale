#pragma once
#include <cstdint>
#include <vector>

template<typename Key, typename Value>
struct DictionaryEntry {
	Key key;        // +0
	Value value;    // +8  ← RETURNED
	int hashCode;   // +16
	int next;       // +20
};

template<typename Key, typename Value>
struct Dictionary {  // Struct from pEric
	void* method_table;
	Array<int>* buckets;
	Array<DictionaryEntry<Key, Value>*>* entries;			// Eric McDonald: The templated type here is actually a DictionaryEntry<Key, Value>, but its structure is highly dependent on the templated types.
	void* comparer;											// Eric McDonald: Can't retrieve this in Ghidra.
	void* keys;												// Eric McDonald: Can't retrieve this in Ghidra.
	void* values;											// Eric McDonald: Can't retrieve this in Ghidra.
	char padding[0x18];
	uint64_t fast_mod_multiplier;
	int count;
	int free_list;
	int free_count;
	int version;

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


	DictionaryEntry<Key, Value>* FindEntry(Key key) {
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

			DictionaryEntry<Key, Value>* entry = entries->getUnsafe(index);

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
			DictionaryEntry<Key, Value>* entry = entries->getUnsafe(i);
			if (entry->hashCode >= 0) { // valid entry
				keysVector.push_back(entry->key);
			}
		}
		return keysVector;
	}
};