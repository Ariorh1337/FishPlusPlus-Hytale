/*
 * Copyright (c) FishPlusPlus.
 */
#include "Speed.h"
#include "core.h"

#include "Events/EventRegister.h"

Speed::Speed() : Feature("Speed") {
	this->speed = this->RegisterSetting<SliderSetting>("Speed", 1.0f, 0.0f, 5.0f);
}

void Speed::OnMoveCycle(DefaultMovementController* dmc, Vector3& offset) {
    dmc->SpeedMultiplier = 1.0f;
    float yawRad = Util::getLocalPlayer()->yawRad;
    float forwardX = -sin(yawRad);
    float forwardZ = -cos(yawRad);

    float strafeX = forwardZ;
    float strafeZ = -forwardX;

    dmc->Velocity.x = 0.0f;
    dmc->Velocity.z = 0.0f;
    offset.x = 0.0f;
    offset.z = 0.0f;
    float currentSpeed = this->speed->GetValue();

    if (InputSystem::IsKeyHeld(SDL_SCANCODE_W))
        offset += Vector3(forwardX * currentSpeed, offset.y, forwardZ * currentSpeed);
    if (InputSystem::IsKeyHeld(SDL_SCANCODE_S))
        offset += Vector3(-forwardX * currentSpeed, offset.y, -forwardZ * currentSpeed);
    if (InputSystem::IsKeyHeld(SDL_SCANCODE_A))
        offset += Vector3(strafeX * currentSpeed, offset.y, strafeZ * currentSpeed);
    if (InputSystem::IsKeyHeld(SDL_SCANCODE_D))
        offset += Vector3(-strafeX * currentSpeed, offset.y, -strafeZ * currentSpeed);
}

bool Speed::CanExecute() {
    return Util::isFullyInitialized();
}

void Speed::Initialize() {
	Util::log("Initialized Speed feature\n");
    RegisterEvent(this);
}	
