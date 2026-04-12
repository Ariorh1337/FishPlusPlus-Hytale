#pragma once

struct ItemLibraryModule {
	char pad_00[0x10];														// 0x0
	GameInstance* gameInstance;											// 0x10
	Dictionary<HytaleString*, void*>* _itemPlayerAnimationsById;		// 0x18 System.Collections.Generic.Dictionary`2[System.String,HytaleClient.Data.Items.ClientItemPlayerAnimations]
	void* DefaultItemPlayerAnimations; 									// 0x20 HytaleClient.Data.Items.ClientItemPlayerAnimations
	Dictionary<HytaleString*, void*>* ItemIcons;						// 0x28 System.Collections.Generic.Dictionary`2[System.String,HytaleClient.Data.Items.ClientIcon]
	Dictionary<HytaleString*, void*>* _items;							// 0x30 System.Collections.Generic.Dictionary`2[System.String,HytaleClient.Data.Items.ClientItemBase]
	Dictionary<HytaleString*, void*>* _modelsByChecksum;				// 0x38 System.Collections.Generic.Dictionary`2[System.String,HytaleClient.Data.BlockyModels.BlockyModel]
	ConcurrentDictionary<HytaleString*, void*>* _animationsByChecksum;	// 0x40 System.Collections.Concurrent.ConcurrentDictionary`2[System.String,HytaleClient.Data.BlockyModels.BlockyAnimation]
	Dictionary<HytaleString*, void*>* ResourceTypes;					// 0x48 System.Collections.Generic.Dictionary`2[System.String,HytaleClient.Data.Items.ClientResourceType]
	Dictionary<HytaleString*, void*>* ClientItemCraftingRecipes;		// 0x50 System.Collections.Generic.Dictionary`2[System.String,HytaleClient.Data.Items.ClientItemCraftingRecipe]
	void* action;														// 0x58 System.Action
	char pad_60[0x10];													// 0x60
	Array<int>* someArray;												// 0x70 System.Int32[]
	Array<Dictionary<HytaleString*, void*>*>* someArray2;				// 0x78 System.Collections.Generic.Dictionary`2+Entry[System.String,HytaleClient.Data.BlockyModels.BlockyModel][]
};