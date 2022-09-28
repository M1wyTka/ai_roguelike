# ai_roguelike
![HW1 drawio (1)](https://user-images.githubusercontent.com/48966303/192884936-a73e78ba-bf40-4c75-8ddf-29da806cc309.png)

# How build with Visual Studio
1. Clone 3rdParty submodules
```
git submodule init
git submodule update
```
2. build bgfx
```
cd 3rdParty/bgfx/
..\bx\tools\bin\windows\genie --with-examples vs2019
```
3. build glfw
```
cd ../glfw/
cmake .
```
4. In properties for projects change windows SDK from 8.1 to 10 (Projects: bgfx, bimg, bx, example-common)
5. For glfw project set Multi-threaded Debug (/MTd)
