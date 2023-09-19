# Survival Maze

3D game developed in OpenGL and C++

## About the game

The player is spawned somewhere in the maze and has 2 minutes and 30 seconds to find the path to the exit.

There are some cubes enemies as cubes random spawned in the maze that decrease
the level of life for the player when they are hit by him.

The player can hit the enemies with some bullets and a hit enemy disappear after 4 seconds. These bullets can be used when the player is in first person mode.

Player will lose the game if there is no time left or if the level of life is decreasing until zero.

## Controls

`W A S D` to move the player in the maze.<br/>
`Right click` to change the camera betweeen third person and first person.<br/>
`Left click/Space` to launch a bullet to an enemy.

## Source code

Source code for current game can be found in folder `\src\lab_m1\tema2`

## Build

### Prerequisites

You need to install CMake to compile the project.

### Steps

From PowerShell on Windows with CMake, use following commands:  

```
mkdir build
cd build
cmake ..
cmake --build .
```

## Run

After building the project, use following command `` .\bin\Debug\GFXFramework `` to run the game.