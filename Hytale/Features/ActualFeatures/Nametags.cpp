/*
 * Copyright (c) FishPlusPlus.
 */
#include "Nametags.h"

#include "core.h"

Nametags::Nametags() : Feature("Nametags") {
	this->NPCTags = this->RegisterSetting<ToggleSetting>("Show NPCs", true);
}

void Nametags::OnRender3D(Renderer3D& renderer3D) {
	SDK::global_mutex.lock();
	std::vector<EntityData> entities = SDK::entities;
	SDK::global_mutex.unlock();

	for (const auto& entity : entities) {
		Vector2 screenPos;
		ValidPtrLoop(entity.entityPtr);
		if (entity.isLocalPlayer)
			continue;

		if (!entity.player && !this->NPCTags->GetValue())
			continue;

		if (!Util::WorldToScreen(entity.entityPtr->RenderPos + Vector3(0.0f, 2.1f, 0.0f), screenPos))
			continue;

		Fonts::Figtree->RenderTextShadow(entity.name, screenPos.x - Fonts::Figtree->getWidth(entity.name) / 2, screenPos.y, 1, Color::White());

	}
}

bool Nametags::CanExecute() {
	if (!Util::IsValidPtr(Util::getLocalPlayer()))
		return false;
	return true;
}
void Nametags::Initialize() {
	Util::log("Initialized Nametags feature\n");
	RegisterEvent(this);
}