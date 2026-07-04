#pragma once

// 1. Include the mixer headers so the compiler knows what a 'Mix_Music' is
#ifdef __has_include
    #if __has_include(<SDL2/SDL_mixer_ext.h>)
        #include <SDL2/SDL_mixer_ext.h>
    #else
        #include <SDL_mixer_ext.h>
    #endif
#endif

// 2. The Prototype: This tells stagemus.cpp that this function exists elsewhere!
void PlayMusic(Mix_Music* music, float tempo, float pitch);