# KnP
Framework for reading and writing memory from an unsigned kernel driver. Uses KdMapper which exploits iqvw64e.sys and maps our unsigned driver into memory. Avoids anti-cheat and anti-virus by embedding and obfuscating the KnPDriver and kdmapper inside the KnPCore application. Also avoids anti-cheat by using self modifying code to edit the KnPDriver's byte code at runtime.

## Build Requirements
```
1. Visual Studio 2019
2. Windows SDK
3. Windows WDK
```
## Examples
- AssaultCube - https://github.com/da772/KnP/tree/AssaultCube
- Gta5 - https://github.com/da772/KnP/tree/Gta5
 
## Dependencies 
- KDMapper - https://github.com/TheCruZ/kdmapper
- zLib - https://zlib.net/

## Disclaimer
*This framework was created for and is to be used for educational purposes only.*
