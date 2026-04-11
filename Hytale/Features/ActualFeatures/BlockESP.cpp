/*
 * Copyright (c) FishPlusPlus.
 */
#include "BlockESP.h"

#include "core.h"

BlockESP::BlockESP() : Feature("BlockESP") {
	this->radius   = this->RegisterSetting<SliderSetting>("Radius", 50.f, 5.f, 500.f);
	this->showName = this->RegisterSetting<ToggleSetting>("Show Name", false);
}

void BlockESP::OnRender3D(Renderer3D& renderer3D) {
	Entity* localPlayer = Util::getLocalPlayer();
	Vector3 playerPos = localPlayer->RenderPos;

	SDK::filteredBlockMutex.lock();
	for (const auto& block : SDK::filteredBlocks) {
		Vector3 blockCenter(block.position.x + 0.5f, block.position.y + 0.5f, block.position.z + 0.5f);
		float dx = blockCenter.x - playerPos.x;
		float dy = blockCenter.y - playerPos.y;
		float dz = blockCenter.z - playerPos.z;
		float distSq = dx * dx + dy * dy + dz * dz;

		if (distSq > this->radius->GetValue() * this->radius->GetValue())
			continue;

		renderer3D.BoxOutline(
			Vector3((int)block.position.x, (int)block.position.y, (int)block.position.z),
			Vector3(1, 1, 1),
			block.color
		);

		if (this->showName->GetValue()) {
			Vector2 screenPos;
			std::string text = Util::string_format("%s (%.1fm)", block.displayName, sqrtf(distSq));
			if (Util::WorldToScreen(blockCenter, screenPos)) {
				Fonts::Figtree->RenderText(
					text,
					screenPos.x - Fonts::Figtree->getWidth(text) / 2,
					screenPos.y,
					1,
					Color::White()
				);
			}
		}
	}
	SDK::filteredBlockMutex.unlock();
}

bool BlockESP::CanExecute() {
	if (Util::app->Stage != AppStage::InGame)
		return false;
	return true;
}

void BlockESP::Initialize() {
	Util::log("Initialized BlockESP feature\n");
	RegisterEvent(this);
}
