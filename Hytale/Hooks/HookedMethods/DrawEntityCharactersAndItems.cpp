/*
 * Copyright (c) FishPlusPlus.
 */
#include "../Hooks.h"

#include <cstdint>
#include <windows.h>

// lazy resolver
template<typename T>
T sub_offset(uintptr_t address) {
    static T fn = nullptr;
    if (!fn)
        fn = reinterpret_cast<T>(address);
    return fn;
}

inline void beginGLContext(void* stateBuffer) {
    sub_offset<void(__fastcall*)(void*)>(SM::beginGLContextAddress)(stateBuffer);
}
inline void endGLContext(void* stateBuffer) {
    sub_offset<void(__fastcall*)(void*)>(SM::endGLContextAddress)(stateBuffer);
}
inline void renderQueueFlush(uint64_t queue, uint64_t renderContext) {
    sub_offset<void(__fastcall*)(uint64_t, uint64_t)>(SM::renderQueueFlushAddress)(queue, renderContext);
}
inline void submitDrawCommands() {
    sub_offset<void(__fastcall*)()>(SM::submitDrawCommandsAddress)();
}

#pragma optimize("", off)
#pragma runtime_checks("", off)
__declspec(safebuffers) __declspec(noinline)
void originalDrawEntityCharactersAndItems(SceneRenderer* _this, bool useOcclusionCulling) {
    uint8_t glStateBuffer[96]{ };
    SceneContext* sceneContext = _this->contextContainer->sceneContext;
    RenderStats* renderStats = _this->renderDevice->renderStats;
    int totalEntityCount = _this->getTotalEntityCount();
    EntityList* entityList = _this->entityList;

    if (useOcclusionCulling) {
        for (int i = 0; i < _this->_entityDrawTaskCount; i++) {
            OcclusionFilterTable* occlusionFilter = _this->occlusionFilter;
            if (i >= entityList->count)
                return;
            EntityDrawTask* entityDrawTask = &entityList->entities[i];
            if (entityDrawTask->EntityLocalId >= occlusionFilter->count)
                return;

            if (occlusionFilter->VisibleOccludees[entityDrawTask->EntityLocalId] == 1) {
                if (*(&SM::g_GlobalStateTableAddress - 1))
                    submitDrawCommands();
                uint64_t uniformMgrPtr = *(uint64_t*) (SM::g_UniformManagerAddress);
                uniformMgrPtr = *(uint64_t*) (uniformMgrPtr + 0x10);
                auto setShaderUniform = (void(__fastcall*)(uint64_t, uint64_t, uint64_t))(*(uint64_t*) (uniformMgrPtr + 0x348));
                uint32_t shaderProgramId = sceneContext->shaderProgramId;
                beginGLContext(glStateBuffer);
                setShaderUniform(shaderProgramId, 0, (uint32_t) i);
                endGLContext(glStateBuffer);

                uint64_t bufferMgrPtr = *(uint64_t*) (SM::g_BufferManagerAddress);
                bufferMgrPtr = *(uint64_t*) (bufferMgrPtr + 8);
                auto bindUniformBufferRange = (void(__fastcall*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))(*(uint64_t*) (bufferMgrPtr + 0x278));

                beginGLContext(glStateBuffer);
                bindUniformBufferRange(35345, sceneContext->uniformBufferId, entityDrawTask->AnimationData.InternalId, entityDrawTask->AnimationDataOffset, entityDrawTask->AnimationDataSize);
                endGLContext(glStateBuffer);
                glBindVertexArray(entityDrawTask->VertexArray.InternalId);
                ++renderStats->drawCallCount;
                renderStats->totalIndicesDrawn += entityDrawTask->DataCount;
                glDrawElements(GL_TRIANGLES, entityDrawTask->DataCount, GL_UNSIGNED_SHORT, 0);
                sceneContext = sceneContext;
            }
        }
    } else {
        for (int i = 0; i < _this->_entityDrawTaskCount; i++) {
            if (*(&SM::g_GlobalStateTableAddress - 1))
                submitDrawCommands();
            if ((uint32_t) i >= entityList->count)
                return;

            EntityDrawTask* entityDrawTask = &entityList->entities[i];

            uint64_t uniformMgrPtr = *(uint64_t*) (SM::g_UniformManagerAddress);
            uniformMgrPtr = *(uint64_t*) (uniformMgrPtr + 0x10);
            auto setShaderUniform = (void(__fastcall*)(uint64_t, uint64_t, uint64_t))(*(uint64_t*) (uniformMgrPtr + 0x348));
            uint32_t shaderProgramId = sceneContext->shaderProgramId;
            beginGLContext(glStateBuffer);
            setShaderUniform(shaderProgramId, 0, (uint32_t) i);
            endGLContext(glStateBuffer);

            uint64_t bufferMgrPtr = *(uint64_t*) (SM::g_BufferManagerAddress);
            bufferMgrPtr = *(uint64_t*) (bufferMgrPtr + 8);
            auto bindUniformBufferRange = (void(__fastcall*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))(*(uint64_t*) (bufferMgrPtr + 0x278));

            beginGLContext(glStateBuffer);
            bindUniformBufferRange(35345, sceneContext->uniformBufferId, entityDrawTask->AnimationData.InternalId, entityDrawTask->AnimationDataOffset, entityDrawTask->AnimationDataSize);
            endGLContext(glStateBuffer);
            glBindVertexArray(entityDrawTask->VertexArray.InternalId);
            ++renderStats->drawCallCount;
            renderStats->totalIndicesDrawn += entityDrawTask->DataCount;
            glDrawElements(GL_TRIANGLES, entityDrawTask->DataCount, GL_UNSIGNED_SHORT, 0);
        }
    }
}
#pragma runtime_checks("", restore)
#pragma optimize("", on)

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
