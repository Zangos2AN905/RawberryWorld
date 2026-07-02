#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Include SDL2 headers
#if __has_include(<SDL2/SDL.h>)
    #include <SDL2/SDL.h>
#else
    #include <SDL.h>
#endif
#if __has_include(<SDL2/SDL_mixer_ext.h>)
    #include <SDL2/SDL_mixer_ext.h>
#else
    #include <SDL_mixer_ext.h>
#endif

#include <SDL_ttf.h>
#include <SDL_image.h>

// Screen dimensions
const int SCREEN_WIDTH = 848;
const int SCREEN_HEIGHT = 480;

int main(int argc, char *argv[]) {
    // 1. Initialize Core SDL (Video and Audio)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    // 2. Initialize SDL_image (PNG support for sprites)
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        fprintf(stderr, "SDL_image missing PNG support: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    // 3. Initialize SDL_mixer (MIDI and SFX)
    // MIX_INIT_MID is used if you are loading standard MIDI files
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer could not initialize: %s\n", Mix_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // 4. Initialize SDL_ttf (Fonts for UI/HUD)
    if (TTF_Init() == -1) {
        fprintf(stderr, "SDL_ttf could not initialize: %s\n", TTF_GetError());
        Mix_CloseAudio();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // 5. Create Window & Hardware Accelerated Renderer
    // SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC ensures smooth, GPU-driven 60fps
    SDL_Window* window = SDL_CreateWindow("RawberryWorld", 
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        goto cleanup;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        goto cleanup;
    }

    // --- GAME LOOP ---
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        // Clear screen to black
        SDL_RenderClear(renderer);

        // Update screen
        SDL_RenderPresent(renderer);
    }

    // --- CLEANUP ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

cleanup:
    TTF_Quit();
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();

    return 0;
}