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

#include <stdio.h>
#include <map>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <GL/glu.h>
#include "game.h"
#include "texture.h"
#include "init.h"
#include "timer.h"
#include "player.h"
#include "mymath.h"
#include "tilemap.h"
#include "enemy.h"
#include "font.h"
#include "bomb.h"
#include "particle.h"
#include "bonus.h"
#include "screenshot.h"
#include "levels.h"
#include "teleport.h"
#include "effects.h"
#include "special_power.h"
#include "soundmusic.h"
#include "trap.h"
#include "bgrounds.h"
#include "mpak.h"
#include "helpers.h"
#include "comments.h"
#include "select_special.h"
#include "hiscore.h"

// Background texture list
int bgrounds[NUM_BGROUNDS];
GLuint cur_bground;

// Fading stuff
// Are we fading? (0 == no, 1 == fade in, 2 == fade out)
int fading;
float fade_amount;


// Lighting
static float light_ambient[] = { 0.75f, 0.75f, 0.75f, 1.0f };
static float light_diffuse1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static float light_diffuse2[] = { 0.5f, 0.5f, 0.5f, 1.0f };
//static float light_diffuse2[] = { 0.3f, 0.8f, 1.0f, 1.0f };
static float light_position1[] = { MAP_W*0.5f, 6.0f, MAP_H*0.75f, 1.0f };
static float light_position2[] = { -MAP_W*0.5f, 2.0f, MAP_H*0.25f, 1.0f };


// Arrays containing currently pressed keys and buttons
std::map<SDL_Keycode, Uint8> key;
Uint8 btn[2][SDL_CONTROLLER_BUTTON_MAX + 4] = {0}; // + 4 so we can pretend analog sticks are d-pads

// Function that returns state of pressed key/button for given player
Uint8 pressed(int key_or_btn, int which_player) {
        if(config.ctl_type[which_player] == KEYBOARD)
                return key[key_or_btn];
        else
                return btn[which_player][key_or_btn];
}

// Kill count
int kill_count;

// Are we playing a two player game?
bool two_players;


// From bonus.cpp
extern int killed_5_diamonds;


// Confirm the ESC or START press
static bool confirm_exit;


// Current fps
static float fps;


// Load background textures
void load_bgrounds() {
	// Prepare the background list
	for(int f=0; f<NUM_BGROUNDS; f++)
		bgrounds[f] = f;

	// Set the lighting
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse2);
	glLightfv(GL_LIGHT2, GL_POSITION, light_position2);
	glEnable(GL_LIGHT2);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
}


// Draw the scene
void draw() {
	//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Set up the viewport (either perspective or ortho)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(!config.perspective_mode)
		glOrtho(-11.0f, 11.0f, -8.0f, 8.0f, -50.0f, 50.0f);
	else {
		gluPerspective(30.0f, 1.333333f, 1, 100);
		glTranslatef(0,0,-32);
	}

	// Draw the background
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	BIND_TEXTURE(cur_bground);
	glDepthMask(GL_FALSE);
	glColor3f(1,1,1);
	glBegin(GL_TRIANGLE_STRIP);
		if(!config.perspective_mode) {
			glTexCoord2f(1,1); glVertex3f(11.0f, 8.0f, -2);
			glTexCoord2f(0,1); glVertex3f(-11.0f, 8.0f, -2);
			glTexCoord2f(1,0); glVertex3f(11.0f, -8.0f, -2);
			glTexCoord2f(0,0); glVertex3f(-11.0f, -8.0f, -2);
		}
		else {
			glTexCoord2f(1,1); glVertex3f(12.2f, 9.1f, -2);
			glTexCoord2f(0,1); glVertex3f(-12.2f, 9.1f, -2);
			glTexCoord2f(1,0); glVertex3f(12.2f, -9.1f, -2);
			glTexCoord2f(0,0); glVertex3f(-12.2f, -9.1f, -2);
		}
	glEnd();
	glDepthMask(GL_TRUE);

	// Rotate to the isometric view
	glRotatef(30.0f, 1.0f, 0,0);
	glRotatef(-45.0f, 0, 1.0f, 0.0f);

	// Center the map around the origin
	glTranslatef(-MAP_W * 0.5f, 0, -MAP_H * 0.5f);


	// Draw the map
	draw_map();

	// Enable alpha blending for sprites
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw the traps
	draw_traps();


	// Enable alpha testing
	glEnable(GL_ALPHA_TEST);

	// Draw the enemies
	draw_enemies();

	// Draw the players
	p1.draw();
	if(two_players)
		p2.draw();

	// Draw the potato men
	draw_potatomen();

	// Draw the bombs
	draw_bombs();

	// Disable alpha blending
	glDisable(GL_ALPHA_TEST);



	// Draw the bonuses
	draw_bonus();

	// Draw the teleports
	draw_teleports();


	// Draw the particles
	draw_particles();

	// Draw the wisps
	draw_wisps();

	// Draw the special powers
	draw_special_power();

	// Draw the player icons when in two player mode
	draw_player_icons();


	// Begin font drawing
	begin_fonts();

	// Draw the FPS amount
	if(config.show_fps && !icon_menu.visible) {
		glColor3f(1,1,1);
		glprintf_center(font1, 0, 0, 9, -25, "FPS: %02.1f", fps);
	}

	// Draw the kill count
	glColor3f(1,1,0.4f);
	char str[10] = "";
	sprintf(str, "%4d", kill_count);
	glprint_num(2.0f, 3.0f - 1.2f*icon_menu.pos, -10, str);

	// Draw the remaining seconds
	sprintf(str, "%2d", num_seconds_left);
	glColor3f(1, 0.8f, 0.4f);
	set_font_scale(0.25f, 0.3f);
	glprint_num(4.7f, 2.6f - 1.2f*icon_menu.pos, -10, str);
	set_font_scale(1,1);


	// Draw the comments
	draw_comments();

	// Draw the special power selection menu
	draw_special_selection();

	// Draw the ESC confirmation
	if(confirm_exit) {
		glLoadIdentity();
		BIND_TEXTURE(0);
		glColor4f(0,0,0, 0.45f);
		glBegin(GL_TRIANGLE_STRIP);
			glVertex3f(4, 3, -2);
			glVertex3f(-4, 3, -2);
			glVertex3f(4,-3,-2);
			glVertex3f(-4,-3,-2);
		glEnd();

		glColor3f(1,1,1);
		set_font_scale(0.4f, 0.6f);
		glprintf_center(font1, 0, 0, 2.0f, -13, "ARE YOU SURE YOU WANT TO EXIT?");
		glprintf_center(font1, 0, 0, 1.1f, -13, "Press ESC/START to exit, or ENTER/A to resume.");
		set_font_scale(1,1);
	}

	// Draw the fade
	if(fade_amount != 0.0f) {
		glLoadIdentity();
		BIND_TEXTURE(0);
		glColor4f(0,0,0,fade_amount);
		glBegin(GL_TRIANGLE_STRIP);
			glVertex3f(4, 3, -2);
			glVertex3f(-4, 3, -2);
			glVertex3f(4,-3,-2);
			glVertex3f(-4,-3,-2);
		glEnd();
	}


	end_fonts();

	// Flush and swap the buffers
	glFlush();
	SDL_GL_SwapWindow(window);
}

// Convert stick position to cardinal direction
int stick_direction(int axis, int value) {
        // Remember stick position
        static int valx = 0;
        static int valy = 0;
        bool cond = (value >= 12288) || (value <= -12288); // 1 if value is outside dead zone
        if((axis % 2) == 0) // horizontal axes are even in SDL_GameControllerAxis
                valx = cond * value;
        else
                valy = cond * -value;
        
        if((valy == 0) && (valx == 0))
                return 0;
        double angle = atan2((double)valy, (double)valx);
        if((angle <= (0.75 * M_PI)) && (angle > (0.25 * M_PI)))
                return STICK_UP;
        if((angle <= (0.25 * M_PI)) && (angle > (-0.25 * M_PI)))
                return STICK_RIGHT;
        if((angle <= (-0.25 * M_PI)) && (angle > (-0.75 * M_PI)))
                return STICK_DOWN;
        else
                return STICK_LEFT;
}

// Handle SDL events
// Return true if we're going to quit
bool handle_event(SDL_Event &event) {

	switch(event.type) {
		case SDL_QUIT:		// Quit
			return true;

		case SDL_KEYDOWN:
			// Update the 'key' array
			key[event.key.keysym.sym] = 1;
			return false;

		case SDL_KEYUP:
			// Update the 'key' array
			key[event.key.keysym.sym] = 0;
			return false;
		
		case SDL_CONTROLLERBUTTONDOWN:
		        // Update the 'btn' array
			btn[event.cbutton.which][event.cbutton.button] = 1;
			return false;
		
		case SDL_CONTROLLERBUTTONUP:
		        // Update the 'btn' array
			btn[event.cbutton.which][event.cbutton.button] = 0;
			return false;
		
		case SDL_CONTROLLERAXISMOTION: // Analog sticks
		        memset(btn[event.caxis.which] + STICK_UP, 0, 4);
		        int dir = stick_direction(event.caxis.axis, event.caxis.value);
		        if(dir)
          	                btn[event.caxis.which][dir] = 1;
		        return false;
	}

	return false;
}


// Start the game
void start_game(bool two_pls) {
	// Initialize
	key.clear();
	memset(btn, 0, sizeof(btn));
	clear_comments();
	clear_map();
	clear_enemies();
	clear_bombs();
	clear_particles();
	clear_bonus();
	clear_traps();
	kill_count = 0;
	two_players = two_pls;

	// Set the players
	p1.clear();
	p2.clear();

	// Init the levels
	init_levels();


	// Fade in
	fading = 1;
	fade_amount = 1.0f;


	// Set up the main loop
	game_paused = false;
	confirm_exit = false;
	timer_count = 0;
	Uint32 fps_counter = 0;
	int frames_drawn = 0;
	Uint32 prev_ticks;
	prev_ticks = SDL_GetTicks();
	Uint32 confirm_time = 0;
	bool main_loop_done = false;
	bool scrshot_down = false;
	bool skip_down = false;
	bool unpause_down = false;
	bool exit_down = false;
	bool finished = false;


	// Main loop
	while(!main_loop_done) {
		// Handle events
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			main_loop_done = handle_event(event);
		}

		// Game loop
		while(timer_count > 0 && !game_paused) {
			// Fade
			if(fading) {
				if(fading == 1) {
					// Fade in
					fade_amount -= 0.01f;
					if(fade_amount <= 0.0f) {
						fading = 0;
						fade_amount = 0.0f;
					}
				}
				else if(fading == 2) {
					// Fade out
					fade_amount += 0.01f;
					if(fade_amount >= 1.0f) {
						fading = 0;
						fade_amount = 1.0f;
						main_loop_done = true;
					}
				}
			}

			// Check keypresses
			// ESC press
			if((key[SDLK_ESCAPE] || btn[0][SDL_CONTROLLER_BUTTON_START] || btn[1][SDL_CONTROLLER_BUTTON_START]) && !fading) {
				if(exit_down == false) {
					exit_down = true;
					if(!confirm_exit) {
						confirm_exit = true;
						// Keep track of the time, so we can restore it
						confirm_time = SDL_GetTicks();
					}
					else {
						// Fade out
						fading = 2;
						fade_amount = 0.0f;
					}
				}
			}
			else
				exit_down = false;

			// ENTER press
			if(confirm_exit) {
				if(key[SDLK_RETURN] || btn[0][SDL_CONTROLLER_BUTTON_A] || btn[1][SDL_CONTROLLER_BUTTON_A]) {
					if(unpause_down == false) {
						unpause_down = true;
						confirm_exit = false;
						if(!killed_5_diamonds)
							level_time += (SDL_GetTicks() - confirm_time);
					}
				}
				else
					unpause_down = false;
			}


			// Screenshot?
			if(key[SDLK_F12] || btn[0][SDL_CONTROLLER_BUTTON_LEFTSHOULDER] || btn[1][SDL_CONTROLLER_BUTTON_LEFTSHOULDER]) {
				if(scrshot_down == false) {
					scrshot_down = true;
					save_screenshot();
				}
			}
			else
				scrshot_down = false;
			
			// Skip currently playing track
			if(key[SDLK_F1] || btn[0][SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] || btn[1][SDL_CONTROLLER_BUTTON_RIGHTSHOULDER]) {
				if(skip_down == false) {
					skip_down = true;
					Mix_HaltMusic();
				}
			}
			else
				skip_down = false;

			if(!confirm_exit) {

				// Handle the level pause
				if(level_pause == LEVEL_PAUSE_BEGIN) {
					if(key[SDLK_RETURN] || btn[0][SDL_CONTROLLER_BUTTON_A] || btn[1][SDL_CONTROLLER_BUTTON_A] || pressed(config.key_shoot[0], 0) || pressed(config.key_shoot[1], 1)) {
						if(!unpause_down) {
							// Start timing the level change
							start_level_timing();
							level_pause = 0;
							unpause_down = true;

							// A little hack to prevent players firing their bombs accidentally,
							// if they press their Shoot button to start the level.
							p1.reload = 10;
							p2.reload = 10;
						}
					}
					else
						unpause_down = false;
				}

				// Do the game stuff
				if(!icon_menu.visible && level_pause != LEVEL_PAUSE_BEGIN && !finished) {
					// Change to the next level if enough time has passed
					// (we don't call this when the Killed-5 diamonds are visible, because
					//  it advances the time, and we don't want that to happen.)
					if(!killed_5_diamonds || level_pause == LEVEL_PAUSE_END) {
						if(!change_level()) {
							// We've finished all the levels.
							// Fade out
							finished = true;
							fading = 2;
							fade_amount = 0.0f;
						}
					}

					// Animate stuff
					move_particles();
					animate_teleports();

					// Update the special powers
					update_special_power();

					// Move the stuff
					p1.move();
					if(two_players)
						p2.move();
					move_enemies();
					move_bombs();
					move_bonus();
					move_traps();
					move_wisps();
					move_potatomen();
				}

				// Do the comments
				move_comments();

				// Handle the special power selection menu
				if(!finished)
					do_special_selection();
			}


			// Advance the fps_counter
			if(config.show_fps) {
				Uint32 ticks_now = SDL_GetTicks();
				Uint32 diff = ticks_now - prev_ticks;
				fps_counter += diff;
				prev_ticks = ticks_now;

				if(fps_counter >= 1000) {
					fps = (float)frames_drawn / (float)(fps_counter/1000.0f);
					frames_drawn = 0;
					fps_counter = 0;
				}
			}

			timer_count--;
		}

		// Draw the scene
		draw();
		frames_drawn++;
	}

	// Free the stuff
	clear_enemies();
	clear_bombs();
	clear_particles();
	clear_bonus();
	clear_traps();
	kill_map();


	// Check if we get to the hiscore list
	HISCORE_LIST &list = (two_players) ? hiscore_2 : hiscore_1;
	int place = list.add_name("null", kill_count);
	if(place != -1) {
		// Get the name and save the list
		list.input_name(place);
		if(two_players)
			list.save(get_hiscore_location(2,true));
		else
			list.save(get_hiscore_location(1,true));
	}
}


