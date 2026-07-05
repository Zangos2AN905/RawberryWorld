#ifndef RENDERING_H
#define RENDERING_H

#if __has_include(<SDL2/SDL.h>)
    #include <SDL2/SDL.h>
#else
    #include <SDL.h>
#endif

const int SCREEN_WIDTH = 848;
const int SCREEN_HEIGHT = 480;

#include "physics.h"

class Level;

namespace Rendering {

    bool InitRendering(SDL_Renderer* renderer);
    void CleanupRendering();

    void SetCamera(const SonicPhysics::CharacterState& player, bool inputDown = false, bool inputUp = false);
    void RenderLevel(const Level& level);
    void RenderPlayer(const SonicPhysics::CharacterState& player, float dt, bool inputDown);

}

#endif
