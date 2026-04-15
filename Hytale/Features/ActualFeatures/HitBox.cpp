/*
 * Copyright (c) FishPlusPlus.
 */
#include "HitBox.h"

HitBox::HitBox() : Feature("Hitbox") {
	this->players = this->RegisterSetting<ToggleSetting>("Players", true);
	this->mobs = this->RegisterSetting<ToggleSetting>("Mobs", true);
};

void HitBox::OnFrame() {
	if (Util::app->Stage != AppStage::InGame)
		return;

	for (EntityData& data : SDK::entities) {
		if (!data.entityPtr)
			continue;

		if (data.isLocalPlayer)
			continue;
		
		Entity* entity = data.entityPtr;

		bool isPlayer = entity->IsAPlayer();
		bool shouldAffect = (isPlayer && this->players->GetValue()) || (!isPlayer && this->mobs->GetValue());

		if (!entityHotboxSave.contains(entity)) {
			entityHotboxSave[entity] = entity->Hitbox;
		}

		if (shouldAffect) {
			Vector3 offset =
				(Util::getLocalPlayer()->Position + Vector3(0, 1, 0)) - entity->Position;

			BoundingBox box;
			box.min = offset - Vector3(0.1, 0, 0.1);
			box.max = offset + Vector3(0.1, 1, 0.1);

			entity->DefaultHitbox = box;
		}
		else {
			entity->DefaultHitbox = entityHotboxSave[entity];
		}
	}
}

void HitBox::OnDeactivate() {
	if (Util::app->Stage != AppStage::InGame)
		return;

	for (auto& [entity, box] : entityHotboxSave) {
		if (entity)
			entity->DefaultHitbox = box;
	}

	entityHotboxSave.clear();
}

void HitBox::Initialize() {
	Util::log("Initialized Hitbox feature\n");
	RegisterEvent(this);
}