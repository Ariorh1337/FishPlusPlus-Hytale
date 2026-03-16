/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

struct Vector3 {
	float x;
	float y;
	float z;

	Vector3& operator=(float value) {
		x = value;
		y = value;
		z = value;
		return *this;
	}

	Vector3& operator=(const Vector3& value) {
		x = value.x;
		y = value.y;
		z = value.z;
		return *this;
	}

	bool operator==(const Vector3& value) const {
		return x == value.x && y == value.y && z == value.z;
	}

	
	Vector3 operator-(const Vector3& value) const {
		return Vector3(x - value.x, y - value.y, z - value.z);
	}
	Vector3 operator+(const Vector3& value) const {
		return Vector3(x + value.x, y + value.y, z + value.z);
	}
	Vector3& operator+=(const Vector3& value) {
		x += value.x;
		y += value.y;
		z += value.z;
		return *this;
	}
	Vector3 operator*(float value) const {
		return Vector3(x * value, y * value, z * value);
	}

	float length() const {
		return sqrtf(this->x * this->x + this->y * this->y + this->z * this->z);
	}

	Vector3 normalized() const {
		auto const length = this->length();
		if (length != 0) {
			auto const inv = 1.0f / length;
			return { this->x * inv, this->y * inv, this->z * inv };
		}

		return *this;
	}
};