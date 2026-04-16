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

Packets block:
Some packets may not work due to the unknown structure of some fields.
Before using that packets - may sure you 'collect' them from send\received original packets. Once you will have something similar to this:
[01:41:32] [SubTypeReg] Learned 'ItemWithAllMetadata'  offset=0x1B164D0  (add to static table)
[01:43:19] [PacketReceiver] Received ApplyKnockback  fields_set=3
you are able to send that packets with the same name and fields.

check Hytale\Features\ActualFeatures\SubTypeRegistry.cpp for known structures

!send-packet {"name":"TeleportToWorldMapPosition","x":1000,"y":1000}
!send-packet {"name":"ClientMovement","absolute_position":{"x":100,"y":64,"z":100}}
!send-packet {"name":"ClientMovement","body_orientation":{"yaw":1.57,"pitch":0},"absolute_position":{"x":0,"y":100,"z":0}}

!receive-packet {"name":"SetGameMode", "game_mode": 1}

etc

For freaking interactions ID:
If this part got broken try to use !dump-interactions and check how it works

Open Chect Packet.
override_root_interaction - feels like same as target_slot
chain_id - this is increasing number from 0 ++
root_interaction - is string with name of interaction, can be found all by !dump-interactions - use INTERACTION~ to resolve it for id
!packet-lab to open packet send window (chat has symbol limits)

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
        "block_position": {"x": 118, "y": 128, "z": 26},
        "proxy_id": {"a":0, "b":0, "c":0, "d":0, "e":0, "f":0, "g":0, "h":0, "i":0, "j":0, "k":0}
      },
      "interaction_data": [
        { "operation_counter": 0, "root_interaction": "INTERACTION~*Empty_Interactions_Use", "state": 0, "block_face": 0, "entity_id": 0, "entered_root_interaction": -2147483648, "placed_block_id": -2147483648, "charge_value": -1, "chaining_index": -1, "flag_index": -1, "generated_u_u_i_d": {"a":0, "b":0, "c":0} },
        { "operation_counter": 1, "root_interaction": "INTERACTION~*Empty_Interactions_Use", "state": 0, "block_face": 5, "entity_id": 0, "block_position": {"x": 118, "y": 128, "z": 26}, "entered_root_interaction": -2147483648, "placed_block_id": -2147483648, "charge_value": -1, "chaining_index": -1, "flag_index": -1, "generated_u_u_i_d": {"a":0, "b":0, "c":0} },
        { "operation_counter": 0, "root_interaction": "INTERACTION~Open_Container", "state": 0, "block_face": 5, "entity_id": 0, "block_position": {"x": 118, "y": 128, "z": 26}, "entered_root_interaction": -2147483648, "placed_block_id": -2147483648, "charge_value": -1, "chaining_index": -1, "flag_index": -1, "generated_u_u_i_d": {"a":0, "b":0, "c":0} },
        { "operation_counter": 2, "root_interaction": "INTERACTION~*Empty_Interactions_Use", "state": 0, "block_face": 0, "entity_id": 0, "entered_root_interaction": -2147483648, "placed_block_id": -2147483648, "charge_value": -1, "chaining_index": -1, "flag_index": -1, "generated_u_u_i_d": {"a":0, "b":0, "c":0} }
      ]
    }
  ]
}

-- ToDO
- clean this ai slop mess?
- offsets for target_slot and chain_id


---

## Discord
[Discord Server](https://discord.gg/4uj596FZ9v)
