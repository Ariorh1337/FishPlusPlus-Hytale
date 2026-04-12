#pragma once

struct ClientBlockType {
	char pad_0000[8]; //0x0000
	HytaleString* Name; //0x0008
	char pad_0010[288]; //0x0010
	uint32_t Id; //0x0130
	char pad_0134[900]; //0x0134
};