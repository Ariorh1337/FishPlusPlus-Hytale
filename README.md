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

TODO: 
-- array of packets?

For freaking interactions ID:
If this part got broken try to use !dump-interactions and check how it works


---

## Discord
[Discord Server](https://discord.gg/4uj596FZ9v)
