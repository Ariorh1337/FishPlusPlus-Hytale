#include "../Hooks.h"

void __fastcall Hooks::hkDrawEntityCharactersAndItems(SceneRenderer* instance, bool useOcclusionCulling) {
    if (!Util::isFullyInitialized())
        return Hooks::oDrawEntityCharactersAndItems(instance, useOcclusionCulling);

    //Render entities through the walls
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, -1500000.0f);
    Hooks::oDrawEntityCharactersAndItems(instance, false);
    glPolygonOffset(1.0f, 1500000.0f);
    glDisable(GL_POLYGON_OFFSET_FILL);


    fboRenderer->bind();

    Hooks::oDrawEntityCharactersAndItems(instance, false);

    fboRenderer->unbind();
}