# I Have No Tomatoes
By [Mika Halttunen](https://mhgames.org/)

Forked from Linux source tarball version 1.55

Original website: https://tomatoes.sourceforge.net/

![In this game, your task is to clear the map of walking tomatoes using bombs and powerups.](screenshot2.png)
![Enemies drop crystals that let you acquire powerups.](screenshot1.png)

## Changes
* Ported to SDL2
* Game controller support (including analog sticks)
* Re-added the ability to skip background music

## Compiling
1. Install dependencies: `SDL2`, `SDL2-mixer`, `SDL2-image`, `mesa-libGLU`
2. Clone this repository and `cd` into it
3. `mkdir obj`
4. `make -j$(nproc)`
