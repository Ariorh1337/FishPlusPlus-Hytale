#pragma once
#include "sdk/BaseDataTypes/Object.h"
#include "Enums.h"	

struct BlockPosition : Object { // Struct from pEric
	int x;
	int y;
	int z;

	BlockPosition(int x, int y, int z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	BlockPosition(Vector3 vec) {
		this->x = (int) vec.x;
		this->y = (int) vec.y;
		this->z = (int) vec.z;
	}
};

struct BlockRotation : Object { // Struct from pEric
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

struct Vector3f : Object { // Struct from pEric
	float x;
	float y;
	float z;

	Vector3f(Vector3 vec) {
		this->x = vec.x;
		this->y = vec.y;
		this->z = vec.z;
	}
};

struct Guuid { // Struct from pEric
	uint32_t a;
	uint16_t b;
	uint16_t c;
	char padding[0x8];

	Guuid(uint32_t a1, uint16_t a2, uint16_t a3) {
		this->a = a1;
		this->b = a2;
		this->c = a3;
	}
};

struct Position : Object { // Struct from pEric
	double x;
	double y;
	double z;

	Position(Vector3 vec) {
		this->x = vec.x;
		this->y = vec.y;
		this->z = vec.z;
	}
};

struct Direction : Object { // Struct from pEric
	float yaw;
	float pitch;
	float roll;

	Direction(Vector3 vec) {
		this->yaw = vec.x;
		this->pitch = vec.y;
		this->roll = vec.z;
	}
};

struct MapChunk : Object {
	int chunkX;
	int chunkY;

	void* image; //MapImage
};