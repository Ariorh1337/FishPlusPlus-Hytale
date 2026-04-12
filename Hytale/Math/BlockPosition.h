#pragma once

struct BlockPosition {
	void* mt = nullptr;

	int x;
	int y;
	int z;

	BlockPosition(int x, int y, int z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
};