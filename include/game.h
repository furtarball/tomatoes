/*************************************************************************

                         "I Have No Tomatoes"
                  Copyright (c) 2004, Mika Halttunen

 This software is provided 'as-is', without any express or implied
 warranty. In no event will the authors be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute
 it freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must
    not claim that you wrote the original software. If you use this
    software in a product, an acknowledgment in the product documentation
    would be appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must
    not be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.


 Mika Halttunen <lsoft@mbnet.fi>

*************************************************************************/

#ifndef GAME_H
#define GAME_H

#include <map>

// Function that returns state of pressed key/button for given player
Uint8 pressed(int key_or_btn, int which_player);

// Arrays containing currently pressed keys and buttons
extern std::map<SDL_Keycode, Uint8> key;
extern Uint8 btn[2][SDL_CONTROLLER_BUTTON_MAX + 4]; // + 4 so we can pretend analog sticks are d-pads
// extend the enum
const int STICK_UP = SDL_CONTROLLER_BUTTON_MAX + 0;
const int STICK_DOWN = SDL_CONTROLLER_BUTTON_MAX + 1;
const int STICK_LEFT = SDL_CONTROLLER_BUTTON_MAX + 2;
const int STICK_RIGHT = SDL_CONTROLLER_BUTTON_MAX + 3;

// Kill count
extern int kill_count;

// Are we playing a two player game?
extern bool two_players;

// Convert stick position to cardinal direction
int stick_direction(int axis, int value);

// Load background textures
void load_bgrounds();

// Start the game
void start_game(bool two_players = false);

#endif


