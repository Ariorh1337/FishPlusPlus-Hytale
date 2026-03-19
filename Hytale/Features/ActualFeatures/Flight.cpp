/*
 * Copyright (c) FishPlusPlus.
 */
#include "Flight.h"

#include "core.h"

Flight::Flight() : Feature("Flight") {
	this->mode = this->RegisterSetting<MultiSetting>("Mode", std::vector<std::string>{"Creative", "Velocity"}, 0);
	this->speed = this->RegisterSetting<SliderSetting>("Speed", 1.0f, 0.0f, 5.0f);
}

void Flight::OnMoveCycle(DefaultMovementController* dmc, Vector3& offset) {
	if (mode->GetValue() == 1) {
		
		dmc->SpeedMultiplier = 1.0f;
		dmc->clientMovementStates.IsFlying = false;
		float yawRad = Util::getLocalPlayer()->yawRad;
		float forwardX = -sin(yawRad);
		float forwardZ = -cos(yawRad);

		float strafeX = forwardZ;
		float strafeZ = -forwardX;

		dmc->Velocity = 0.0f;
		offset = 0.0f;
		if (!Util::ShouldInteractWithGame())
			return;
		if (InputSystem::IsKeyHeld(SDL_SCANCODE_W))
			offset += Vector3(forwardX * this->speed->GetValue(), offset.y, forwardZ * this->speed->GetValue());
		if (InputSystem::IsKeyHeld(SDL_SCANCODE_S)) 
			offset += Vector3(-forwardX * this->speed->GetValue(), offset.y, -forwardZ * this->speed->GetValue());
		if (InputSystem::IsKeyHeld(SDL_SCANCODE_A))
			offset += Vector3(strafeX * this->speed->GetValue(), offset.y, strafeZ * this->speed->GetValue());
		if (InputSystem::IsKeyHeld(SDL_SCANCODE_D))
			offset += Vector3(-strafeX * this->speed->GetValue(), offset.y, -strafeZ * this->speed->GetValue());
		if (InputSystem::IsKeyHeld(SDL_SCANCODE_SPACE))
			offset.y = speed->GetValue();
		if (InputSystem::IsKeyHeld(SDL_SCANCODE_LSHIFT))
			offset.y = -speed->GetValue();
	} else {
		dmc->SpeedMultiplier = this->speed->GetValue();
		dmc->clientMovementStates.IsFlying = true;
	}
}

void Flight::OnDeactivate() {
	ValidPtrVoid(Util::getGameInstance());
	ValidPtrVoid(Util::GetMovementController());
	DefaultMovementController* dmc = Util::GetMovementController();
	dmc->clientMovementStates.IsFlying = false;
}

bool Flight::CanExecute() {
	ValidPtrBool(Util::getLocalPlayer());
	ValidPtrBool(Util::GetMovementController());
}

void Flight::Initialize() {
	Util::log("Initialized Flight feature\n");
	EventRegister::DoMoveCycleEvent.Subscribe([&](DefaultMovementController* dmc, Vector3& dir) {
		if (this->IsActive())
			this->OnMoveCycle(dmc, dir);
		});
}