# Shape Blaster using Urho3D
Inspired by Shape Blaster XNA (https://gamedevelopment.tutsplus.com/series/cross-platform-vector-shooter-xna--gamedev-10559), but with slightly different gameplay.

# Requirements
* Urho3D (https://urho3d.github.io/) (Tested with Urho3D version 1.7)

# Build
## Urho3D

If you are familiar with Urho3D, just skip this section.

The easiest way to get Urho3D SDK is to download a precompiled version from https://sourceforge.net/projects/urho3d/files/Urho3D/.

Otherwise, download Urho3D, unpack it and build SDK as follows:

    mkdir urho3d_build
	cd urho3d_build
    cmake -G "<GENERATOR_NAME>" -DCMAKE_INSTALL_PREFIX=<PATH_TO_INSTALL_SDK> <URHO3D_SOURCE_PATH>
    cmake --build . --target install

Change `<GENERATOR_NAME>`, `<PATH_TO_INSTALL_SDK>` and `<URHO3D_SOURCES_PATH>` to suit your environment.
	
## Game

After successfully building Urho3D, build the game the very same way.
Just ensure that compilation options (architecture (x32/x64) and build type (debug/release) for Windows/VS solutions) are identical both for Urho3D SDK and game itself.

    mkdir build
	cd build
    cmake -G "<GENERATOR_NAME>" -DURHO3D_HOME=<PATH_TO_SDK> <GAME_SRC_PATH>
	cmake --build .

Look for executable in `build/bin` directory.

# Controls

| Control          | Action                      |
|------------------|-----------------------------|
| WASD             | move player                 |
| SPACE            | start game / respawn player |
| ESC              | quit game                   |
| Mouse LMB        | shoot                       |

# Gameplay

Collect (catch) Wanderers (red things), avoid Seekers and Black holes.
