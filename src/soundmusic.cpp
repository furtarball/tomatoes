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

#include <stdlib.h>
#include <stdio.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "game.h"
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include "mymath.h"
#include "init.h"
#include "mpak.h"
#include "comments.h"
#include "timer.h"
#include "soundmusic.h"

// Current music module
Mix_Music *music_mod = NULL;

// Music files array
char music_files[MAX_MUSIC][256];
int num_music_files;
int cur_music;

// Sound array
Mix_Chunk *sounds[NUM_SOUNDS];


// Shuffle the playlist
void shuffle_playlist() {
	if(!num_music_files)
		return;

	cur_music = 0;

	// Shuffle the list
	for(int i = num_music_files - 1; i > 0; i--) {
		int pos = RAND(0, i - 1);
		char swap[256];
		strcpy(swap, music_files[i]);
		strcpy(music_files[i], music_files[pos]);
		strcpy(music_files[pos], swap);
	}
}


// Search for the music files
void search_music() {
	DIR *dp;
	dirent *ep;

	// Search files from the music directory
	dp = opendir(MUSIC_DIR);
	if(!dp || !config.sound || !config.music_vol) {
		// No files found, or the sound is turned off
		num_music_files = 0;
		return;
	}

	// Clear music file list
	num_music_files = 0;
	cur_music = 0;
	for(int f=0; f<MAX_MUSIC; f++)
		strcpy(music_files[f], "");

	// Start searching
	while((ep = readdir(dp)) ) {
		if(num_music_files >= MAX_MUSIC-1) {
			printf("Warning: Too many music files in '%s' directory!\n", MUSIC_DIR);
			break;
		}

		// Check the extension
		char ext[3];
		char name[256] = "";
		strcpy(name, ep->d_name);
		int len = strlen(name);
		if(len > 3) {
			ext[0] = name[len-3];
			ext[1] = name[len-2];
			ext[2] = name[len-1];
			if(toupper(ext[0]) == 'M' && toupper(ext[1]) == 'O' && toupper(ext[2]) == 'D') {
				// Found MOD
				strcpy(music_files[num_music_files], name);
				num_music_files++;
			}

			if(toupper(ext[0]) == 'S' && toupper(ext[1]) == '3' && toupper(ext[2]) == 'M') {
				// Found S3M
				strcpy(music_files[num_music_files], name);
				num_music_files++;
			}

			if(toupper(ext[1]) == 'X' && toupper(ext[2]) == 'M') {
				// Found XM
				strcpy(music_files[num_music_files], name);
				num_music_files++;
			}

			if(toupper(ext[1]) == 'I' && toupper(ext[2]) == 'T') {
				// Found IT
				strcpy(music_files[num_music_files], name);
				num_music_files++;
			}

			if(toupper(ext[0]) == 'O' && toupper(ext[1]) == 'G' && toupper(ext[2]) == 'G') {
				// Found OGG
				strcpy(music_files[num_music_files], name);
				num_music_files++;
			}
		}
	}

	closedir(dp);
	shuffle_playlist();
}


// If the current music has finished, start playing another
void music_finished() {
	static bool f1_key_down = false;

	if(!config.sound || !num_music_files || !config.music_vol)
		return;
	
	if(f1_key_down == false) {
		// Play the current song from the playlist
		play_music(music_files[cur_music]);

		cur_music++;
		if(cur_music > num_music_files-1)
			shuffle_playlist();
		f1_key_down = true;
	}
	if(!key[SDLK_F1])
		f1_key_down = false;

}


// This helper function loads a sound and stores it to the sound array
static int cur_sound = 0;
void load_sound(char *file) {
	if(cur_sound > NUM_SOUNDS-1)
		error_msg("load_sounds():\nTrying to load too many sounds!\nNUM_SOUNDS is defined as %d.\n", NUM_SOUNDS);

	// First try to load from the override directory
	char soundfile[128] = "";
	sprintf(soundfile, "%s%s", pakfile.override_dir, file);
	FILE *check = fopen(soundfile, "rb");
	if(check) {
		// The file exists, load it
		fclose(check);
		sounds[cur_sound] = Mix_LoadWAV(soundfile);
	}
	else {
		// The file doesn't exist in the override directory
		// try to load it from the pakfile.
		FILE *fp = pakfile.open_file(file);
		SDL_RWops *rw = SDL_RWFromFP(fp,0); 
		sounds[cur_sound] = Mix_LoadWAV_RW(rw,0);
		SDL_FreeRW(rw);
		fclose(fp);
	}

	// Check for errors
	if(!sounds[cur_sound])
		error_msg("load_sound():\nUnable to load a sound from %s!\nError: %s\n",file,Mix_GetError());
	cur_sound++;
}


// Initialize the SDL_Mixer
void init_mixer() {
	if(!config.sound)
		return;
	
	// Initialize the Mixer	
	if(Mix_OpenAudio(config.sound_freq, AUDIO_S16, 2, 4096))	// Frequency, 16 bit sound, channels (stereo), buffer size
		error_msg("Unable to open the audio device!\nError: %s\n", Mix_GetError());
	
	// Load the sounds
	load_sound("snd_appear.wav");
	load_sound("snd_bomb.wav");
	load_sound("snd_explo.wav");
	load_sound("snd_bonus1.wav");
	load_sound("snd_bonus2.wav");
	load_sound("snd_bonus3.wav");
	load_sound("snd_die1.wav");
	load_sound("snd_die2.wav");
	load_sound("snd_die3.wav");
	load_sound("snd_die4.wav");
	load_sound("snd_die5.wav");
	load_sound("snd_die6.wav");
	load_sound("snd_levelteleport.wav");
	load_sound("snd_wildfire.wav");
	load_sound("snd_teleport.wav");
	load_sound("snd_trap.wav");
	load_sound("snd_lightning.wav");
	load_sound("snd_wisp.wav");
	load_sound("snd_jump.wav");
	load_sound("snd_potatoman.wav");
	load_sound("snd_potatoman2.wav");
	load_sound("snd_turn.wav");
	load_sound("snd_flowerpower.wav");
	load_sound("snd_kick.wav");
	load_sound("snd_killed5.wav");
	load_sound("snd_menu1.wav");
	load_sound("snd_menu2.wav");
	load_sound("snd_finish.wav");

	// Search for music files
	search_music();
	
	// Set the volume
	Mix_Volume(-1,config.sound_vol);

	// Start playing the music
	if(num_music_files) {
		play_music(music_files[0]);
		cur_music++;
		if(cur_music > num_music_files-1)
			cur_music = 0;
	}
	
	// Tell Mixer what to do when the music stops
	Mix_HookMusicFinished(&music_finished);
}


// Play music
void play_music(char *file) {
	if(!config.sound || !config.music_vol)
		return;

	game_paused = true;

	char str[256] = "";
	sprintf(str, "%s%s", MUSIC_DIR, file);
	if(music_mod){
		Mix_HaltMusic();
		Mix_FreeMusic(music_mod);
	}
	music_mod = Mix_LoadMUS(str);
	if(!music_mod)
		error_msg("play_music():\nUnable to play music from '%s'!\nError: %s\n", file, Mix_GetError());

	// Play
	Mix_PlayMusic(music_mod, 0);

	// Add the comment
	add_comment(COL_DEFAULT, "Playing \"%s\"...", file);

	// Set the volume
	Mix_VolumeMusic(config.music_vol);

	game_paused = false;
}


// Play a sound
void play_sound(int sound, bool random_freq) {
	if(!config.sound)
		return;

	// Play the sound
	Mix_PlayChannel(-1, sounds[sound], 0);
	//int channel = Mix_PlayChannel(-1, sounds[sound], 0);

	// Random the frequency
	// This doesn't work in SDL!
/*	if(random_freq) {
		int freq;
		int freq_change[2];
//		freq = FSOUND_GetFrequency(channel);

		freq_change[0] = (int)((float)freq * 0.3f);
		freq_change[1] = (int)((float)freq * 0.1f);

		// Upload the frequency to the sample
//		FSOUND_SetFrequency(channel, freq+RAND(-freq_change[0], freq_change[1]));
	}
*/
}
