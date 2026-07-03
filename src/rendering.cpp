#include "rendering.h"
#include "physics.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace Rendering {

constexpr float PLAYER_SCALE = 1.25f;

// World bounds (in physics units)
constexpr int WORLD_MIN_X = -3000;
constexpr int WORLD_MAX_X = 3000;
constexpr int WORLD_MIN_Y = -1000;
constexpr int WORLD_MAX_Y = 1000;

inline int clamp(int value, int min, int max) {
    return std::max(min, std::min(max, value));
}

struct Sprite {
    SDL_Texture* texture = nullptr;
    int frameWidth = 0;
    int frameHeight = 0;
    int frameCount = 0;
    int currentFrame = 0;
    float frameTimer = 0.0f;
    float frameDuration = 0.8f;
    bool looping = true;
};

std::unordered_map<std::string, Sprite> sprites;
SDL_Renderer* gRenderer = nullptr;
int cameraX = 0;
int cameraY = 0;

bool LoadSprite(const std::string& name, const std::string& path, int frameCount, float frameDuration, bool looping);

bool InitRendering(SDL_Renderer* renderer) {
    gRenderer = renderer;
    
    if (!LoadSprite("idle", "assets/sprites/characters/rawberry/idle_strip8.png", 8, 2.85f, true)) return false;
    if (!LoadSprite("walk", "assets/sprites/characters/rawberry/walk_strip6.png", 6, 2.85f, true)) return false;
    if (!LoadSprite("jump", "assets/sprites/characters/rawberry/jump_strip7.png", 7, 2.85f, false)) return false;
    if (!LoadSprite("dash", "assets/sprites/characters/rawberry/dash_strip10.png", 10, 0.05f, false)) return false;
    if (!LoadSprite("crouch", "assets/sprites/characters/rawberry/crouch_strip17.png", 17, 2.85f, true)) return false;
    if (!LoadSprite("hurt", "assets/sprites/characters/rawberry/hurt_strip1.png", 1, 0.1f, false)) return false;
    if (!LoadSprite("hurtground", "assets/sprites/characters/rawberry/hurtground_strip1.png", 1, 0.1f, false)) return false;
    if (!LoadSprite("downhurt", "assets/sprites/characters/rawberry/downhurt_strip1.png", 1, 0.1f, false)) return false;
    if (!LoadSprite("fstrong", "assets/sprites/characters/rawberry/fstrong_strip10.png", 10, 0.08f, false)) return false;
    if (!LoadSprite("ustrong", "assets/sprites/characters/rawberry/ustrong_strip9.png", 9, 0.08f, false)) return false;
    
    return true;
}

bool LoadSprite(const std::string& name, const std::string& path, int frameCount, float frameDuration, bool looping) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        return false;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(gRenderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        return false;
    }
    
    int texW, texH;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
    
    Sprite sprite;
    sprite.texture = texture;
    sprite.frameWidth = texW / frameCount;
    sprite.frameHeight = texH;
    sprite.frameCount = frameCount;
    sprite.frameDuration = frameDuration;
    sprite.looping = looping;
    
    sprites[name] = std::move(sprite);
    return true;
}

void UpdateAnimation(float dt) {
    for (auto& pair : sprites) {
        Sprite& sprite = pair.second;
        if (sprite.frameCount > 1) {
            sprite.frameTimer += dt;
            if (sprite.frameTimer >= sprite.frameDuration) {
                sprite.frameTimer = 0.0f;
                sprite.currentFrame++;
                if (sprite.currentFrame >= sprite.frameCount) {
                    if (sprite.looping) {
                        sprite.currentFrame = 0;
                    } else {
                        sprite.currentFrame = sprite.frameCount - 1;
                    }
                }
            }
        }
    }
}

void SetAnimation(const std::string& name, bool reset) {
    auto it = sprites.find(name);
    if (it != sprites.end()) {
        if (reset || it->second.currentFrame >= it->second.frameCount) {
            it->second.currentFrame = 0;
            it->second.frameTimer = 0.0f;
        }
    }
}

void RenderPlayer(const SonicPhysics::CharacterState& player, float dt, bool inputDown) {
    std::string animName = "idle";
    bool resetAnim = false;
    
    if (!player.grounded) {
        if (player.velocityY < 0) {
            animName = "jump";
        } else {
            animName = "jump";
        }
    } else if (player.spindashing) {
        animName = "dash";
    } else if (player.rolling) {
        animName = "dash";
    } else if (inputDown && std::abs(player.groundSpeed) < 0.5f) {
        animName = "crouch";
    } else if (std::abs(player.groundSpeed) > 0.5f) {
        animName = "walk";
    } else {
        animName = "idle";
    }
    
    static std::string lastAnim = "idle";
    if (animName != lastAnim) {
        resetAnim = true;
        lastAnim = animName;
    }
    
    SetAnimation(animName, resetAnim);
    UpdateAnimation(dt);
    
    auto it = sprites.find(animName);
    if (it == sprites.end()) return;
    
    const Sprite& sprite = it->second;
    
    int screenX = static_cast<int>(player.x) - cameraX + SCREEN_WIDTH / 2;
    int screenY = static_cast<int>(SCREEN_HEIGHT / 2 + player.y) - cameraY;
    
    SDL_Rect srcRect = {
        sprite.currentFrame * sprite.frameWidth,
        0,
        sprite.frameWidth,
        sprite.frameHeight
    };
    
    SDL_Rect dstRect = {
        screenX - static_cast<int>(sprite.frameWidth * PLAYER_SCALE / 2),
        screenY - static_cast<int>(sprite.frameHeight * PLAYER_SCALE),
        static_cast<int>(sprite.frameWidth * PLAYER_SCALE),
        static_cast<int>(sprite.frameHeight * PLAYER_SCALE)
    };
    
    SDL_RendererFlip flip = (player.direction < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(gRenderer, sprite.texture, &srcRect, &dstRect, 0, nullptr, flip);
}

void SetCamera(int x, int y) {
    // Clamp camera to world bounds (convert screen center to world coords)
    int halfScreenW = SCREEN_WIDTH / 2;
    int halfScreenH = SCREEN_HEIGHT / 2;
    
    cameraX = clamp(x, WORLD_MIN_X + halfScreenW, WORLD_MAX_X - halfScreenW);
    cameraY = clamp(y, WORLD_MIN_Y + halfScreenH, WORLD_MAX_Y - halfScreenH);
}

void RenderGroundIndicator() {
    // Ground is at y=0 in physics coordinates
    // Convert to screen coordinates
    int groundScreenY = static_cast<int>(SCREEN_HEIGHT / 2 - 0.0f) - cameraY;
    
    // Only draw if ground is visible on screen
    if (groundScreenY >= 0 && groundScreenY <= SCREEN_HEIGHT) {
        // Draw filled ground area (below ground line)
        SDL_SetRenderDrawColor(gRenderer, 80, 80, 80, 255);
        
        int worldLeft = cameraX - SCREEN_WIDTH / 2;
        int worldRight = cameraX + SCREEN_WIDTH / 2;
        
        SDL_Rect groundRect = {
            0,
            groundScreenY,
            SCREEN_WIDTH,
            SCREEN_HEIGHT - groundScreenY
        };
        SDL_RenderFillRect(gRenderer, &groundRect);
        
        // Draw ground line on top
        SDL_SetRenderDrawColor(gRenderer, 100, 100, 100, 255);
        SDL_RenderDrawLine(gRenderer, 0, groundScreenY, SCREEN_WIDTH, groundScreenY);
        
        // Draw small tick marks every 100 units
        SDL_SetRenderDrawColor(gRenderer, 60, 60, 60, 255);
        for (int wx = (worldLeft / 100) * 100; wx <= worldRight; wx += 100) {
            int sx = wx - cameraX + SCREEN_WIDTH / 2;
            if (sx >= 0 && sx <= SCREEN_WIDTH) {
                SDL_RenderDrawLine(gRenderer, sx, groundScreenY - 5, sx, groundScreenY + 5);
            }
        }
    }
}

void CleanupRendering() {
    for (auto& pair : sprites) {
        if (pair.second.texture) {
            SDL_DestroyTexture(pair.second.texture);
        }
    }
    sprites.clear();
}

} // namespace Rendering