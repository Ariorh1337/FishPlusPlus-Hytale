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
		//Util::log("Initialized hkDrawEntityCharactersAndItems globals: qword_2937CA8=0x%llX, qword_2937CB0=0x%llX, off_2924E78=0x%llX\n", qword_2937CA8, qword_2937CB0, off_2924E78);
    }


    u64 a1 = (u64) _this;

    u64 rbx = a1;

    u64 rsi = *(u64*) (*(u64*) (rbx + 0x38) + 0x30);
    u64 rdi = *(u64*) (*(u64*) (rbx + 0x30) + 0x10);

    u32 r14d = *(u32*) (rbx + 0x20C)
        + *(u32*) (rbx + 0x210)
        + *(u32*) (rbx + 0x214);

    uint8_t stack[96]{ };

    auto FAIL = [&](u64 a, u64 b) {
        sub_B36260(a, b);
        __debugbreak();
    };

    // =========================================================
    // if (!flag)
    // =========================================================
    if (!flag) {
        r14d = 0;

    loc_4DE186:
        if (r14d >= *(u32*) (rbx + 0x1B8))
            goto loc_4DE399;

    loc_4DDFD8:
        if (*(u64*) (off_2924E78 - 8))
            sub_14763A4();

    loc_4DDFF4:
        {
            u64 rcx = *(u64*) (qword_2937CA8);
            rcx = *(u64*) (rcx + 0x10);

            u32 v = *(u32*) (rsi + 0x84);

            auto fn = *(void(__fastcall***)(u64, u64, u64))(rcx);
            auto call = (void(__fastcall*)(u64, u64, u64))(*(u64*) (rcx + 0x348));

            sub_14C7B80(stack);
            call(v, 0, r14d);
            sub_14C7BD0(stack);
        }

        {
            u64 list = *(u64*) (rbx + 0x70);
            if (r14d >= *(u32*) (list + 8))
                FAIL(list, flag);

            u64 entry = list + (u64) r14d * 0x90 + 0x10;

            u32 a = *(u32*) (entry + 0x74);
            u32 b = *(u32*) (entry + 0x78);
            u16 c = *(u16*) (entry + 0x7C);

            u32 ctx = *(u32*) (rsi + 0x74 + 4);

            u64 tbl = *(u64*) (qword_2937CB0);
            tbl = *(u64*) (tbl + 8);

            auto call = (void(__fastcall*)(u64, u64, u64, u64, u64))(*(u64*) (tbl + 0x278));

            sub_14C7B80(stack);
            call(0x8A11, ctx, a, b, c);
            sub_14C7BD0(stack);
        }

        {
            u64 list = *(u64*) (rbx + 0x70);
            if (r14d >= *(u32*) (list + 8))
                FAIL(list, flag);

            u32 val = *(u32*) (list + (u64) r14d * 0x90 + 0x7C);

            auto call = *(void(__fastcall**)(u64))(rdi + 0x250);

            sub_14C7B80(stack);
            call(val);
            sub_14C7BD0(stack);
        }

        {
            u64 list = *(u64*) (rbx + 0x70);
            if (r14d >= *(u32*) (list + 8))
                FAIL(list, flag);

            u32 count = *(u32*) (list + (u64) r14d * 0x90 + 0x80);

            *(u32*) (rdi + 0x438) += 1;
            *(u32*) (rdi + 0x43C) += count;

            auto call = *(void(__fastcall**)(u64, u64, u64, u64))(rdi + 0x3E0);

            sub_14C7B80(stack);
            call(4, count, 0x1403, 0);
            sub_14C7BD0(stack);
        }

        r14d++;
        goto loc_4DE186;
    }

    // =========================================================
    // else (flag == true)
    // =========================================================

    {
        u32 r15d = 0;

    loc_4DE38C:
        if (r15d >= *(u32*) (rbx + 0x1B8))
            goto loc_4DE399;

    loc_4DE1A4:
        {
            u64 table = *(u64*) (rbx + 0xD8);
            u64 list  = *(u64*) (rbx + 0x70);

            if (r15d >= *(u32*) (list + 8))
                FAIL(list, flag);

            u64 idx = *(u16*) (list + (u64) r15d * 0x90 + 0x9C) + r14d;

            if (idx >= *(u32*) (table + 8))
                FAIL(table, flag);

            if (*(u32*) (table + 0x10 + idx * 4) != 1) {
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

            u32 v = *(u32*) (rsi + 0x84);

            auto call = (void(__fastcall*)(u64, u64, u64))(*(u64*) (rcx + 0x348));

            sub_14C7B80(stack);
            call(v, 0, r15d);
            sub_14C7BD0(stack);
        }

        {
            u64 list = *(u64*) (rbx + 0x70);
            if (r15d >= *(u32*) (list + 8))
                FAIL(list, flag);

            u64 entry = list + (u64) r15d * 0x90 + 0x10;

            u32 a = *(u32*) (entry + 0x74);
            u32 b = *(u32*) (entry + 0x78);
            u16 c = *(u16*) (entry + 0x7C);

            u32 ctx = *(u32*) (rsi + 0x74 + 4);

            u64 tbl = *(u64*) (qword_2937CB0);
            tbl = *(u64*) (tbl + 8);

            auto call = (void(__fastcall*)(u64, u64, u64, u64, u64))(*(u64*) (tbl + 0x278));

            sub_14C7B80(stack);
            call(0x8A11, ctx, a, b, c);
            sub_14C7BD0(stack);
        }

        {
            u64 list = *(u64*) (rbx + 0x70);
            if (r15d >= *(u32*) (list + 8))
                FAIL(list, flag);

            u32 val = *(u32*) (list + (u64) r15d * 0x90 + 0x7C);

            auto call = *(void(__fastcall**)(u64))(rdi + 0x250);

            sub_14C7B80(stack);
            call(val);
            sub_14C7BD0(stack);
        }

        {
            u64 list = *(u64*) (rbx + 0x70);
            if (r15d >= *(u32*) (list + 8))
                FAIL(list, flag);

            u32 count = *(u32*) (list + (u64) r15d * 0x90 + 0x80);

            *(u32*) (rdi + 0x438) += 1;
            *(u32*) (rdi + 0x43C) += count;

            auto call = *(void(__fastcall**)(u64, u64, u64, u64))(rdi + 0x3E0);

            sub_14C7B80(stack);
            call(4, count, 0x1403, 0);
            sub_14C7BD0(stack);
        }

        r15d++;
        goto loc_4DE38C;
    }

loc_4DE399:
    return;





    /*

    //Render entities through the walls
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, -1500000.0f);
    Hooks::oDrawEntityCharactersAndItems(instance, false);
    glPolygonOffset(1.0f, 1500000.0f);
    glDisable(GL_POLYGON_OFFSET_FILL);


    fboRenderer->bind();

    Hooks::oDrawEntityCharactersAndItems(instance, false);

    fboRenderer->unbind();*/
}