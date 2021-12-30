# Assault Cube
External cheats for Assault Cube v1.2
- Esp
- Unlimited ammo
- Unlimited health

Created to demonstrate KnPCore framework

![image](https://user-images.githubusercontent.com/8605626/147782382-7682af25-c4ad-4a39-8796-029c4589ec15.png)

## Build Instructions
```
1. Open KnP.sln (VS2019)
2. Build KnP/Package (x64, Release) (Requires Windows SDK and Windows WDK)
3. Build AssaultCube
```

## Run Instructions
```
1. Open AssaultCube v1.2
2. Run AssaultCube.exe as Administrator
```

# KnP
Framework for reading and writing memory from an unsigned kernel driver. Uses KdMapper which exploits iqvw64e.sys and maps our unsigned driver into memory. Avoids anti-cheat and anti-virus by embedding and obfuscating the KnPDriver and kdmapper inside the KnPCore application. Also avoids anti-cheat by using self modifying code to edit the KnPDriver's byte code at runtime.



 
## Dependencies 
- KDMapper - https://github.com/TheCruZ/kdmapper
- zLib - https://zlib.net/

## Disclaimer
*This framework was created for and is to be used for educational purposes only.*
