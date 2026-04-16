Examples of packets:

# Server to Client (S2C)

## Inject a game mode change into the local client
```json
{"name":"SetGameMode","game_mode":1}
```

# Client to Server (C2S)

## Teleport (would not work without creative)
```json
{"name":"TeleportToWorldMapPosition","x":1000,"y":64,"z":1000}
```

## Movement with nested position
```json
{"name":"ClientMovement","absolute_position":{"x":100,"y":64,"z":100},"body_orientation":{"yaw":1.57,"pitch":0}}
```

## Collect `*Plant_Crop_Wheat_Block_Eternal_State_Definitions_StageFinal` example on x:115 y:128 z:19

```json
{
    "name": "SyncInteractionChains",
    "updates": [
        {
            "data": {
                "block_position": {
                    "x": 115,
                    "y": 128,
                    "z": 19
                },
                "entity_id": -1,
                "target_slot": "AUTO~HOTBAR_SLOT",
                "proxy_id": { "a": 0, "b": 0, "c": 0, "d": 0, "e": 0, "f": 0, "g": 0, "h": 0, "i": 0, "j": 0, "k": 0 }
            },
            "interaction_data": [
                {
                    "progress": 0,
                    "operation_counter": 0,
                    "root_interaction": "INTERACTION~*Empty_Interactions_Use",
                    "total_forks": 0,
                    "entity_id": 0,
                    "entered_root_interaction": "AUTO~HOTBAR_SLOT",
                    "placed_block_id": "AUTO~HOTBAR_SLOT",
                    "charge_value": -1,
                    "chaining_index": -1,
                    "flag_index": -1,
                    "raycast_distance": 0,
                    "next_label": 0,
                    "state": 0,
                    "block_face": 0,
                    "movement_direction": 0,
                    "apply_force_state": 0,
                    "generated_u_u_i_d": { "a": 0, "b": 0, "c": 0, "d": 0, "e": 0, "f": 0, "g": 0, "h": 0, "i": 0, "j": 0, "k": 0 }
                },
                {
                    "block_position": {
                        "x": 115,
                        "y": 128,
                        "z": 19
                    },
                    "progress": 0,
                    "operation_counter": 1,
                    "root_interaction": "INTERACTION~*Empty_Interactions_Use",
                    "total_forks": 0,
                    "entity_id": 0,
                    "entered_root_interaction": "AUTO~HOTBAR_SLOT",
                    "placed_block_id": "AUTO~HOTBAR_SLOT",
                    "charge_value": -1,
                    "chaining_index": -1,
                    "flag_index": -1,
                    "raycast_distance": 0,
                    "next_label": 0,
                    "state": 0,
                    "block_face": 5,
                    "movement_direction": 0,
                    "apply_force_state": 0,
                    "generated_u_u_i_d": { "a": 0, "b": 0, "c": 0, "d": 0, "e": 0, "f": 0, "g": 0, "h": 0, "i": 0, "j": 0, "k": 0 }
                },
                {
                    "block_position": {
                        "x": 115,
                        "y": 128,
                        "z": 19
                    },
                    "progress": 0,
                    "operation_counter": 0,
                    "root_interaction": "INTERACTION~**Plant_Crop_Wheat_Block_Eternal_State_Definitions_StageFinal_Interactions_Use",,
                    "total_forks": 0,
                    "entity_id": 0,
                    "entered_root_interaction": "AUTO~HOTBAR_SLOT",
                    "placed_block_id": "AUTO~HOTBAR_SLOT",
                    "charge_value": -1,
                    "chaining_index": -1,
                    "flag_index": -1,
                    "raycast_distance": 0,
                    "next_label": 0,
                    "state": 0,
                    "block_face": 5,
                    "movement_direction": 0,
                    "apply_force_state": 0,
                    "generated_u_u_i_d": { "a": 0, "b": 0, "c": 0, "d": 0, "e": 0, "f": 0, "g": 0, "h": 0, "i": 0, "j": 0, "k": 0 }
                },
                {
                    "progress": 0,
                    "operation_counter": 2,
                    "root_interaction": "INTERACTION~*Empty_Interactions_Use",
                    "total_forks": 0,
                    "entity_id": 0,
                    "entered_root_interaction": "AUTO~HOTBAR_SLOT",
                    "placed_block_id": "AUTO~HOTBAR_SLOT",
                    "charge_value": -1,
                    "chaining_index": -1,
                    "flag_index": -1,
                    "raycast_distance": 0,
                    "next_label": 0,
                    "state": 0,
                    "block_face": 0,
                    "movement_direction": 0,
                    "apply_force_state": 0,
                    "generated_u_u_i_d": { "a": 0, "b": 0, "c": 0, "d": 0, "e": 0, "f": 0, "g": 0, "h": 0, "i": 0, "j": 0, "k": 0 }
                }
            ],
            "active_hotbar_slot": "AUTO~HOTBAR_SLOT",
            "active_utility_slot": -1,
            "active_tools_slot": -1,
            "override_root_interaction": "AUTO~HOTBAR_SLOT",
            "equip_slot": "AUTO~HOTBAR_SLOT",
            "chain_id": "AUTO~CHAIN_ID",
            "operation_base_index": 0,
            "initial": true,
            "desync": false,
            "interaction_type": 5,
            "state": 0
        }
    ]
}
```

## Open container example on x:118 y:128 z:26

```json
{
  "name": "SyncInteractionChains",
  "dump": false,
  "updates": [
    {
      "active_hotbar_slot": "AUTO~HOTBAR_SLOT",
      "active_utility_slot": -1,
      "active_tools_slot": -1,
      "initial": true,
      "desync": false,
      "override_root_interaction": "AUTO~HOTBAR_SLOT",
      "interaction_type": 5,
      "equip_slot": "AUTO~HOTBAR_SLOT",
      "chain_id": "AUTO~CHAIN_ID",
      "data": {
        "entity_id": -1,
        "target_slot": "AUTO~HOTBAR_SLOT",
        "block_position": {"x": 118, "y": 128, "z": 26}
      },
      "interaction_data": [
        { "operation_counter": 0, "root_interaction": "INTERACTION~*Empty_Interactions_Use", "state": 0, "block_face": 0, "entity_id": 0, "entered_root_interaction": "AUTO~HOTBAR_SLOT", "placed_block_id": "AUTO~HOTBAR_SLOT", "charge_value": -1, "chaining_index": -1, "flag_index": -1 },
        { "operation_counter": 1, "root_interaction": "INTERACTION~*Empty_Interactions_Use", "state": 0, "block_face": 5, "entity_id": 0, "block_position": {"x": 118, "y": 128, "z": 26}, "entered_root_interaction": "AUTO~HOTBAR_SLOT", "placed_block_id": "AUTO~HOTBAR_SLOT", "charge_value": -1, "chaining_index": -1, "flag_index": -1 },
        { "operation_counter": 0, "root_interaction": "INTERACTION~Open_Container",           "state": 0, "block_face": 5, "entity_id": 0, "block_position": {"x": 118, "y": 128, "z": 26}, "entered_root_interaction": "AUTO~HOTBAR_SLOT", "placed_block_id": "AUTO~HOTBAR_SLOT", "charge_value": -1, "chaining_index": -1, "flag_index": -1 },
        { "operation_counter": 2, "root_interaction": "INTERACTION~*Empty_Interactions_Use", "state": 0, "block_face": 0, "entity_id": 0, "entered_root_interaction": "AUTO~HOTBAR_SLOT", "placed_block_id": "AUTO~HOTBAR_SLOT", "charge_value": -1, "chaining_index": -1, "flag_index": -1 }
      ]
    }
  ]
}
```