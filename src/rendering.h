#ifndef RENDERING_H
#define RENDERING_H

#include <SDL2/SDL.h>
#include "physics.h"

namespace Rendering {
    const int SCREEN_WIDTH = 848;
    const int SCREEN_HEIGHT = 480;

    bool InitRendering(SDL_Renderer* renderer);
    void RenderPlayer(const SonicPhysics::CharacterState& player, float dt, bool inputDown);
    void RenderGroundIndicator();
    void SetCamera(int x, int y);
    void CleanupRendering();
}

#endif