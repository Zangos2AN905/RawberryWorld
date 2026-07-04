// Plays music using the SDL2 Mixer X library.
// Can change the tempo and/or pitch of the music in real-time.
#include "music.h"
#include <stdio.h>

#ifdef __has_include
    #if __has_include(<SDL2/SDL_mixer_ext.h>)
        #include <SDL2/SDL_mixer_ext.h>
    #else
        #include <SDL_mixer_ext.h>
    #endif
#endif // <--- 1. FIXED: Closed the preprocessor block

// 2. FIXED: Turned parameters into standard C++ syntax (float tempo, float pitch)
void PlayMusic(Mix_Music* music, float tempo, float pitch) {
    // Play music using the SDL2 Mixer X library (-1 loops indefinitely)
    if (Mix_PlayMusic(music, -1) == -1) {
        fprintf(stderr, "Failed to play music: %s\n", Mix_GetError());
        return;
    }

    // 3. FIXED: Using actual Mixer X API functions to change audio parameters
    // Note: This works brilliantly out of the box for MIDI, Tracker files (MOD/IT/XM), and Chiptunes!
    Mix_SetMusicTempo(music, (double)tempo);
    Mix_SetMusicPitch(music, (double)pitch);
}