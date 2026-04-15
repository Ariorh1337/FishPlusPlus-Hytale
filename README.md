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

!send-packet {"name":"TeleportToWorldMapPosition","x":1000,"y":1000}
Before using - place `packet_descriptors.json` next to HytaleClient.exe (AppData\Roaming\Hytale\install\release\package\game\latest\Client)
Right now - no support for packets with 'ptr' data type in fields (example: no support for ClientPlaceBlock)

## Discord
[Discord Server](https://discord.gg/4uj596FZ9v)
