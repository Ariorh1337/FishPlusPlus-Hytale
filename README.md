# FishPlusPlus

Only for use on anarchy servers such as 6b6t. This was not created or developed with servers in mind that do not allow the use of external software.

DO NOT Use this on servers that do not allow external software.

I do not condone, recommend or want you to use on servers where using external software is not permitted.

## How to build/use

### Requirements:
[Visual Studio 2026](https://visualstudio.microsoft.com/downloads/)  
MSVC

### How to build
Open solution file (.slnx) in VS 2026  
Select build config: Release  
Click Build/Build Solution or use the keybind ctrl + shift + B  

### How to use
Inject with a LoadLibrary injector or manual map injector such as extreme injector

## Commands
Theres a couple of different commands you can use. Just type these in the chat like shown with your input inbetween <>  
!tp \<x\> \<y\> \<z\>  
!config save \<name\>  
!config load \<name\>  

Configs are stored in your Hytale game folder usually in  
>AppData\Roaming\Hytale\install\release\package\game\latest\Client\Fish++

There is also a separate config that gets saved every 2 minutes. It will not overwrite the ones you manually save

---

## Packet Lab

The Packet Lab lets you build and send (or inject) any game packet from a JSON description.

### Commands

| Command | Description |
|---|---|
| `!packet-lab` | Open the Packet Lab window |
| `!dump-interactions` | Dump all interaction IDs to a file in the game folder |

The Packet Lab window has three buttons: **Send C2S**, **Receive S2C**, **Trace: OFF/ON**.

### JSON schema

```json
{
  "name": "PacketStructName",
  "field_name": value,
  "nested_ptr_field": { "sub_field": value },
  "array_field": [ { "elem_field": value } ]
}
```

Special values:
- `"INTERACTION~InteractionName"` — resolved to the matching integer interaction ID at send time
- `"dump": true` — hex-dump the constructed packet bytes before sending (useful for debugging)

### Examples

Paste any of these into the Packet Lab editor and press the appropriate button.

**C2S — teleport**
```json
{"name":"TeleportToWorldMapPosition","x":1000,"y":64,"z":1000}
```

**C2S — movement with nested position**
```json
{"name":"ClientMovement","absolute_position":{"x":100,"y":64,"z":100},"body_orientation":{"yaw":1.57,"pitch":0}}
```

**S2C — inject a game mode change into the local client**
```json
{"name":"SetGameMode","game_mode":1}
```

**C2S — SyncInteractionChains (open container)**
Example that opens a container at block (118, 128, 26):

```json
{
  "name": "SyncInteractionChains",
  "dump": true,
  "updates": [
    {
      "active_hotbar_slot": 8,
      "active_utility_slot": -1,
      "active_tools_slot": -1,
      "initial": true,
      "desync": false,
      "override_root_interaction": -2147483648,
      "interaction_type": 5,
      "equip_slot": 8,
      "chain_id": 17,
      "data": {
        "entity_id": -1,
        "target_slot": -2147483648,
        "block_position": {"x": 118, "y": 128, "z": 26}
      },
      "interaction_data": [
        { "operation_counter": 0, "root_interaction": "INTERACTION~*Empty_Interactions_Use", "state": 0, "block_face": 0, "entity_id": 0, "entered_root_interaction": -2147483648, "placed_block_id": -2147483648, "charge_value": -1, "chaining_index": -1, "flag_index": -1 },
        { "operation_counter": 1, "root_interaction": "INTERACTION~*Empty_Interactions_Use", "state": 0, "block_face": 5, "entity_id": 0, "block_position": {"x": 118, "y": 128, "z": 26}, "entered_root_interaction": -2147483648, "placed_block_id": -2147483648, "charge_value": -1, "chaining_index": -1, "flag_index": -1 },
        { "operation_counter": 0, "root_interaction": "INTERACTION~Open_Container",           "state": 0, "block_face": 5, "entity_id": 0, "block_position": {"x": 118, "y": 128, "z": 26}, "entered_root_interaction": -2147483648, "placed_block_id": -2147483648, "charge_value": -1, "chaining_index": -1, "flag_index": -1 },
        { "operation_counter": 2, "root_interaction": "INTERACTION~*Empty_Interactions_Use", "state": 0, "block_face": 0, "entity_id": 0, "entered_root_interaction": -2147483648, "placed_block_id": -2147483648, "charge_value": -1, "chaining_index": -1, "flag_index": -1 }
      ]
    }
  ]
}
```

Notes on `SyncInteractionChains`:
- `chain_id` — monotonically increasing integer, start from last seen + 1
- `root_interaction` — interaction name string; use `INTERACTION~Name` and it resolves automatically. Run `!dump-interactions` if you need the exact name.
- `override_root_interaction` / `target_slot` — set to `INT_MIN` (-2147483648) when unused

### Discovering unknown field structures

Some packet fields reference types whose MethodTable offsets are not yet confirmed.
When a field can't be built you'll see:

```
[PacketSender] MISSING MethodTable for 'TypeName' — not in SubTypeRegistry::Initialize()
```

Enable `!trace` and play normally to let the runtime scanner learn them:

```
[SubTypeReg] Learned 'TypeName'  offset=0x1BXXXXX  (add to static table)
```

Copy the offset into `SubTypeRegistry::Initialize()` in
`Hytale/Features/ActualFeatures/SubTypeRegistry.cpp`.
Known offsets are listed there with verification notes.


---

## Discord
[Discord Server](https://discord.gg/4uj596FZ9v)
