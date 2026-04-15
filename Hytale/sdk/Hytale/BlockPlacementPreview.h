#pragma once

struct BlockPlacementPreview {
	char pad_0x00[0x8];                                 //0x00
	GameInstance* gameInstance;                         //0x08
	Entity* entity; 									//0x10 HytaleClient.InGame.Modules.Entities.Entity

	// 0x63 IsPreviewVisible
};