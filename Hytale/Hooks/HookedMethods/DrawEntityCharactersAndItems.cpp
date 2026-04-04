/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"





#include <cstdint>
#include <windows.h>

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;

// lazy resolver
template<typename T>
T sub_offset(uintptr_t offset) {
    static T fn = nullptr;
    if (!fn)
        fn = reinterpret_cast<T>(gameBase + offset);
    return fn;
}


// extern globals you must set
uint64_t qword_2937CA8;    // assign externally
uint64_t qword_2937CB0;    // assign externally
uint64_t off_2924E78;      // assign externally

inline void sub_14C7B80(void* x) { sub_offset<void(__fastcall*)(void*)>(0x14C7B80)(x); }
inline void sub_14C7BD0(void* x) { sub_offset<void(__fastcall*)(void*)>(0x14C7BD0)(x); }
inline void sub_B36260(u64 a, u64 b) { sub_offset<void(__fastcall*)(u64, u64)>(0xB36260)(a, b); }
inline void sub_14763A4() { sub_offset<void(__fastcall*)()>(0x14763A4)(); }

void __fastcall Hooks::hkDrawEntityCharactersAndItems(SceneRenderer* _this, bool flag) {
	if (!Util::isFullyInitialized())
		return Hooks::oDrawEntityCharactersAndItems(_this, flag);

	if (qword_2937CA8 == 0 || qword_2937CB0 == 0 || off_2924E78 == 0) {
		qword_2937CA8 = (uint64_t)(gameBase + 0x2937CA8);
		qword_2937CB0 = (uint64_t)(gameBase + 0x2937CB0);
		off_2924E78 = (uint64_t)(gameBase + 0x2924E78);
	}

	ContextData* contextData = (ContextData*)(_this->sceneContext->contextData);
	RenderDevice* renderDevice = _this->renderDevice;
	DrawList* drawList = _this->drawList;

	u32 totalCount = _this->unk_20C + _this->unk_210 + _this->unk_214;

	uint8_t stack[96]{ };

	auto FAIL = [&](u64 a, u64 b) {
		sub_B36260(a, b);
		__debugbreak();
	};

	// =========================================================
	// if (!flag)
	// =========================================================
	if (!flag) {
		u32 index = 0;

	loc_4DE186:
		if (index >= _this->drawCount)
			goto loc_4DE399;

	loc_4DDFD8:
		if (*(u64*) (off_2924E78 - 8))
			sub_14763A4();

	loc_4DDFF4:
		{
			u64 rcx = *(u64*) (qword_2937CA8);
			rcx = *(u64*) (rcx + 0x10);

			u32 v = contextData->shaderParam;

			auto call = (void(__fastcall*)(u64, u64, u64))(*(u64*) (rcx + 0x348));

			sub_14C7B80(stack);
			call(v, 0, index);
			sub_14C7BD0(stack);
		}

		{
			if (index >= drawList->size)
				FAIL((u64)drawList, flag);

			DrawListEntry& entry = drawList->entries[index];

			u32 ctx = contextData->contextValue;

			u64 tbl = *(u64*) (qword_2937CB0);
			tbl = *(u64*) (tbl + 8);

			auto call = (void(__fastcall*)(u64, u64, u64, u64, u64))(*(u64*) (tbl + 0x278));

			sub_14C7B80(stack);
			call(0x8A11, ctx, entry.param_a, entry.param_b, entry.param_c);
			sub_14C7BD0(stack);
		}

		{
			if (index >= drawList->size)
				FAIL((u64)drawList, flag);

			DrawListEntry& entry = drawList->entries[index];

			auto call = *(void(__fastcall**)(u64))(renderDevice + 0x250);

			sub_14C7B80(stack);
			call(entry.param_c);
			sub_14C7BD0(stack);
		}

		{
			if (index >= drawList->size)
				FAIL((u64)drawList, flag);

			DrawListEntry& entry = drawList->entries[index];

			renderDevice->drawCallCount += 1;
			renderDevice->vertexCount += entry.count;

			auto call = *(void(__fastcall**)(u64, u64, u64, u64))(renderDevice + 0x3E0);

			sub_14C7B80(stack);
			call(4, entry.count, 0x1403, 0);
			sub_14C7BD0(stack);
		}

		index++;
		goto loc_4DE186;
	}

	// =========================================================
	// else (flag == true)
	// =========================================================

	{
		u32 r15d = 0;

	loc_4DE38C:
		if (r15d >= _this->drawCount)
			goto loc_4DE399;

	loc_4DE1A4:
		{
			FilterTable* filterTable = _this->filterTable;

			if (r15d >= drawList->size)
				FAIL((u64)drawList, flag);

			DrawListEntry& entry = drawList->entries[r15d];
			u64 idx = entry.filterIndex + totalCount;

			if (idx >= filterTable->size)
				FAIL((u64)filterTable, flag);

			if (filterTable->filters[idx] != 1) {
				r15d++;
				goto loc_4DE38C;
			}
		}

		if (*(u64*) (off_2924E78 - 8))
			sub_14763A4();

	loc_4DE200:
		{
			u64 rcx = *(u64*) (qword_2937CA8);
			rcx = *(u64*) (rcx + 0x10);

			u32 v = contextData->shaderParam;

			auto call = (void(__fastcall*)(u64, u64, u64))(*(u64*) (rcx + 0x348));

			sub_14C7B80(stack);
			call(v, 0, r15d);
			sub_14C7BD0(stack);
		}

		{
			if (r15d >= drawList->size)
				FAIL((u64)drawList, flag);

			DrawListEntry& entry = drawList->entries[r15d];

			u32 ctx = contextData->contextValue;

			u64 tbl = *(u64*) (qword_2937CB0);
			tbl = *(u64*) (tbl + 8);

			auto call = (void(__fastcall*)(u64, u64, u64, u64, u64))(*(u64*) (tbl + 0x278));

			sub_14C7B80(stack);
			call(0x8A11, ctx, entry.param_a, entry.param_b, entry.param_c);
			sub_14C7BD0(stack);
		}

		{
			if (r15d >= drawList->size)
				FAIL((u64)drawList, flag);

			DrawListEntry& entry = drawList->entries[r15d];

			auto call = *(void(__fastcall**)(u64))(renderDevice + 0x250);

			sub_14C7B80(stack);
			call(entry.param_c);
			sub_14C7BD0(stack);
		}

		{
			if (r15d >= drawList->size)
				FAIL((u64)drawList, flag);

			DrawListEntry& entry = drawList->entries[r15d];

			renderDevice->drawCallCount += 1;
			renderDevice->vertexCount += entry.count;

			auto call = *(void(__fastcall**)(u64, u64, u64, u64))(renderDevice + 0x3E0);

			sub_14C7B80(stack);
			call(4, entry.count, 0x1403, 0);
			sub_14C7BD0(stack);
		}

		r15d++;
		goto loc_4DE38C;
	}

loc_4DE399:
	return;
}