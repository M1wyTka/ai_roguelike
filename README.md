Learning materials for the course "AI for videogames" based on simple roguelike mechanics.
* w1 - FSM
* w2 - Behaviour Trees + blackboard
* w3 - Utility functions
* w4 - Emergent behaviour
* w5 - Goal Oriented Action Planning

## Dependencies
This project uses:
* bgfx for week1 project
* raylib for week2 and later project
* flecs for ECS

## Building

To build you first need to update submodules:
```
git submodule sync
git submodule update --init --recursive
```

Then you need to build using cmake:
```
cmake -B build
cmake --build build
```


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

