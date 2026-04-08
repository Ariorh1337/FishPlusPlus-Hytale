#pragma once

template<typename Key, typename Value>
struct Dictionary {
	char pad_00[0x8];					// 0x00
	Array<int>* buckets;
	Array<void*>* entries;  // Eric McDonald: The templated type here is actually a DictionaryEntry<Key, Value>, but its structure is highly dependent on the templated types.
	void* comparer;  // Eric McDonald: Can't retrieve this in Ghidra.
	void* keys;  // Eric McDonald: Can't retrieve this in Ghidra.
	void* values;  // Eric McDonald: Can't retrieve this in Ghidra.
	char padding[0x18];
	uint64_t fast_mod_multiplier;
	int count;
	int free_list;
	int free_count;
	int version;
};
