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
- `"INTERACTION~<Name>"` — resolved to the matching integer interaction ID at send time
- `"AUTO~CHAIN_ID"` — resolved to the next valid chain_id (auto-tracked from incoming packets)
- `"AUTO~HOTBAR_SLOT"` — resolved to the currently active hotbar slot index
- `"dump": true` — hex-dump the constructed packet bytes before sending

### Examples

Paste any of these into the Packet Lab editor and press the appropriate button.

Check ./Packet-Lab.md for packet examples

Notes on `SyncInteractionChains`:
- `chain_id` — use `AUTO~CHAIN_ID`, it stays in sync with what the server has seen
- `active_hotbar_slot` / `equip_slot` — use `AUTO~HOTBAR_SLOT` for the active slot
- `root_interaction` — use `INTERACTION~Name`; run `!dump-interactions` for the exact name
- `override_root_interaction` / `target_slot` — `-2147483648` (INT_MIN) when unused

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
