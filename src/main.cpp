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

#include "physics.h"
#include "rendering.h"

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

    // Initialize physics
    SonicPhysics::InitPhysics();
    SonicPhysics::SetPlayerPosition(0.0f, 0.0f);
    SonicPhysics::SetGrounded(true);

    // Initialize rendering
    if (!Rendering::InitRendering(renderer)) {
        fprintf(stderr, "Failed to initialize rendering\n");
        goto cleanup;
    }

    // --- GAME LOOP ---
    bool isRunning = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();

    bool inputLeft = false;
    bool inputRight = false;
    bool inputJump = false;
    bool inputDown = false;

    while (isRunning) {
        Uint32 currentTime = SDL_GetTicks();
        int deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                    case SDLK_a:
                        inputLeft = true;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        inputRight = true;
                        break;
                    case SDLK_SPACE:
                    case SDLK_UP:
                    case SDLK_w:
                        inputJump = true;
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        inputDown = true;
                        break;
                    case SDLK_ESCAPE:
                        isRunning = false;
                        break;
                }
            } else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                    case SDLK_a:
                        inputLeft = false;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        inputRight = false;
                        break;
                    case SDLK_SPACE:
                    case SDLK_UP:
                    case SDLK_w:
                        inputJump = false;
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        inputDown = false;
                        break;
                }
            }
        }

        // Update physics
        SonicPhysics::UpdatePhysics(inputLeft, inputRight, inputJump, inputDown, deltaTime);

        // Get player state for rendering
        const SonicPhysics::CharacterState& player = SonicPhysics::GetPlayerState();

        // Update camera to follow player
        Rendering::SetCamera(static_cast<int>(player.x), static_cast<int>(player.y));

        // Clear screen to black
        SDL_RenderClear(renderer);

        // Render ground indicator
        Rendering::RenderGroundIndicator();

        // Render player
        Rendering::RenderPlayer(player, deltaTime / 1000.0f * 60.0f, inputDown);

        // Update screen
        SDL_RenderPresent(renderer);
    }

    // --- CLEANUP ---
    Rendering::CleanupRendering();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

cleanup:
    TTF_Quit();
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();

    return 0;
}