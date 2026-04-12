#pragma once

enum Rotation {
	None,
	Ninety,
	OneEighty,
	TwoSeventy
};

struct BlockRotation {
	void* mt = nullptr;

	Rotation rotationYaw = Rotation::None;
	Rotation rotationPitch = Rotation::None;
	Rotation rotationRoll = Rotation::None;

	BlockRotation(Rotation yaw, Rotation pitch, Rotation roll) {
		this->rotationYaw = yaw;
		this->rotationPitch = pitch;
		this->rotationRoll = roll;
	}

	BlockRotation() {
		this->rotationYaw = Rotation::None;
		this->rotationPitch = Rotation::None;
		this->rotationRoll = Rotation::None;
	}
};