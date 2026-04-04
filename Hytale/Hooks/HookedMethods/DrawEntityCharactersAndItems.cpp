/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"

#include <cstdint>
#include <windows.h>

#pragma optimize("", off)
#pragma runtime_checks("", off)

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

__declspec(safebuffers) __declspec(noinline)
void originalDrawEntityCharactersAndItems(SceneRenderer* _this, bool useOcclusionCulling) {
    if (qword_2937CA8 == 0 || qword_2937CB0 == 0 || off_2924E78 == 0) {
        qword_2937CA8 = (uint64_t) (gameBase + 0x2937CA8);
        qword_2937CB0 = (uint64_t) (gameBase + 0x2937CB0);
        off_2924E78 = (uint64_t) (gameBase + 0x2924E78);
        //Util::log("Initialized hkDrawEntityCharactersAndItems globals: qword_2937CA8=0x%llX, qword_2937CB0=0x%llX, off_2924E78=0x%llX\n", qword_2937CA8, qword_2937CB0, off_2924E78);
    }

    uint64_t a1 = (uint64_t) _this;

    uint8_t v39[96]{ };
    uint64_t v3 = *(uint64_t*) (*(uint64_t*) (a1 + 0x38) + 0x30);
    uint64_t v38 = v3;
    uint64_t result = *(uint64_t*) (a1 + 0x30);
    uint64_t v37 = *(uint64_t*) (result + 0x10);
    int v5 = *(uint32_t*) (a1 + 0x214) + *(uint32_t*) (a1 + 0x1B8) + *(uint32_t*) (a1 + 0x20C);
    uint32_t v29;
    uint64_t v8;

    if (useOcclusionCulling) {
        for (int i = 0; i < *(uint32_t*) (a1 + 0x1B8); i = (uint32_t) (i + 1)) {
            result = *(uint64_t*) (a1 + 0xD8);
            v8 = *(uint64_t*) (a1 + 0x70);
            if ((uint32_t) i >= *(uint32_t*) (v8 + 8))
                return;
            uint64_t v16 = 0x90 * i;
            v8 = v5 + (uint32_t) * (uint16_t*) (v8 + 0x90 * i + 0x9C);
            if ((uint32_t) v8 >= *(uint32_t*) (result + 8))
                return;
            if (*(uint32_t*) (result + 4 * v8 + 16) == 1) {
                uint64_t SetValueAddr = *(uint64_t*) (qword_2937CA8);
                SetValueAddr = *(uint64_t*) (SetValueAddr + 0x10);
                auto SetValue = (void(__fastcall*)(uint64_t, uint64_t, uint64_t))(*(uint64_t*) (SetValueAddr + 0x348));
                uint32_t v28 = *(uint32_t*) (v3 + 0x84);
                sub_14C7B80(v39);
                SetValue(v28, 0, (uint32_t) i);
                sub_14C7BD0(v39);
                uint64_t list = *(uint64_t*) (a1 + 0x70);
                if ((uint32_t) i >= *(uint32_t*) (list + 8))
                    return;
                uint64_t v18 = list + v16 + 0x10;
                uint64_t SetBufferRangeAddr = *(uint64_t*) (qword_2937CB0);
                SetBufferRangeAddr = *(uint64_t*) (SetBufferRangeAddr + 8);
                auto SetBufferRange = (void(__fastcall*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))(*(uint64_t*) (SetBufferRangeAddr + 0x278));
                uint16_t v24 = *(uint16_t*) (v18 + 0x7C);
                uint64_t v26 = *(uint32_t*) (v18 + 0x78);
                uint64_t v36 = *(uint32_t*) (v38 + 0x78);
                uint64_t v32 = *(uint32_t*) (v18 + 0x74);
                sub_14C7B80(v39);
                SetBufferRange(35345, v36, v32, v26, v24);
                sub_14C7BD0(v39);
                v8 = *(uint64_t*) (a1 + 0x70);
                if ((uint32_t) i >= *(uint32_t*) (v8 + 8)
                    || (v29 = *(uint32_t*) (v8 + v16 + 0x7C),
                        glBindVertexArray(v29),
                        list = *(uint64_t*) (a1 + 0x70),
                        (uint32_t) i >= *(uint32_t*) (list + 8))) {
                    Util::log("Fail\n");
                }

                uint32_t v21 = *(uint32_t*) (list + v16 + 0x80);
                ++*(uint32_t*) (v37 + 0x438);
                *(uint32_t*) (v37 + 0x43C) += v21;
                uint32_t v30 = v21;

                glDrawElements(GL_TRIANGLES, v30, GL_UNSIGNED_SHORT, 0);
                v3 = v38;
            }
        }
    } else {
        int v6 = 0;
        while (v6 < *(uint32_t*) (a1 + 0x1B8)) {
            uint64_t SetValueAddr = *(uint64_t*) (qword_2937CA8);
            SetValueAddr = *(uint64_t*) (SetValueAddr + 0x10);
            auto SetValue = (void(__fastcall*)(uint64_t, uint64_t, uint64_t))(*(uint64_t*) (SetValueAddr + 0x348));
            uint32_t v28 = *(uint32_t*) (v3 + 0x84);
            sub_14C7B80(v39);
            SetValue(v28, 0, (uint32_t) v6);
            sub_14C7BD0(v39);
            uint64_t list = *(uint64_t*) (a1 + 0x70);
            if ((uint32_t) v6 >= *(uint32_t*) (list + 8))
                return;
            uint64_t v9 = 0x90 * v6;
            uint64_t v10 = list + 0x90 * v6 + 0x10;
            uint64_t SetBufferRangeAddr = *(uint64_t*) (qword_2937CB0);
            SetBufferRangeAddr = *(uint64_t*) (SetBufferRangeAddr + 8);
            auto SetBufferRange = (void(__fastcall*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))(*(uint64_t*) (SetBufferRangeAddr + 0x278));
            uint16_t v24 = *(uint16_t*) (v10 + 0x7C);
            uint64_t v26 = *(uint32_t*) (v10 + 0x78);
            uint64_t v36 = *(uint32_t*) (v38 + 0x78);
            uint64_t v32 = *(uint32_t*) (v10 + 0x74);
            sub_14C7B80(v39);
            SetBufferRange(35345, v36, v32, v26, v24);
            sub_14C7BD0(v39);
            v8 = *(uint64_t*) (a1 + 0x70);
            if ((uint32_t) v6 >= *(uint32_t*) (v8 + 8))
                return;
            v29 = *(uint32_t*) (v8 + v9 + 0x7C);
            glBindVertexArray(v29);
            list = *(uint64_t*) (a1 + 0x70);
            if ((uint32_t) v6 >= *(uint32_t*) (list + 8))
                Util::log("Fail\n");

            uint32_t v21 = *(uint32_t*) (list + v9 + 0x80);
            ++*(uint32_t*) (v37 + 0x438);
            *(uint32_t*) (v37 + 0x43C) += v21;
            uint32_t v30 = v21;
            glDrawElements(GL_TRIANGLES, v30, GL_UNSIGNED_SHORT, 0);
            v6 = (uint32_t) (v6 + 1);
            v3 = v38;
        }
    }
}

void __fastcall Hooks::hkDrawEntityCharactersAndItems(SceneRenderer* _this, bool flag) {
    if (!Util::isFullyInitialized())
        return Hooks::oDrawEntityCharactersAndItems(_this, flag);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, -1500000.0f);
    originalDrawEntityCharactersAndItems(_this, false);
    glPolygonOffset(1.0f, 1500000.0f);
    glDisable(GL_POLYGON_OFFSET_FILL);

    fboRenderer->bind();
    originalDrawEntityCharactersAndItems(_this, false);
    fboRenderer->unbind();
}
