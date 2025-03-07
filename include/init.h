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

#ifndef INIT_H
#define INIT_H

#include "config.h"

// Keep a global pointer to the window and OpenGL context
struct SDL_Window;
extern SDL_Window *window;
typedef void *SDL_GLContext;
extern SDL_GLContext glctx;

// Keep a global pointer to the config
extern CONFIG config;

// Keep a global pointer to the data file
struct MPAK_FILE;
extern MPAK_FILE pakfile;

// Keep a global pointer to the controllers
struct _SDL_GameController;
typedef struct _SDL_GameController SDL_GameController;
extern SDL_GameController *pads[2];

// Keep a global string with a path to the executable
extern char* path;

// Initialize SDL and OpenGL
void init_sdl_and_gl();

// Display an error message and quit
void error_msg(const char *msg, ...);
// Same as above, but don't quit and don't show an error popup
void error_msg_nonfatal(const char *msg, ...);

#endif


