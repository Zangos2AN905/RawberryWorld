#include "rendering.h"
#include "physics.h"
#if __has_include(<SDL2/SDL.h>)
    #include <SDL2/SDL.h>
#else
    #include <SDL.h>
#endif
#if __has_include(<SDL2/SDL_image.h>)
    #include <SDL2/SDL_image.h>
#else
    #include <SDL_image.h>
#endif
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
    int loopStart = 0;    // First frame of loop range
    int loopEnd = 0;      // Last frame (inclusive) of loop range
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
    sprite.loopStart = 0;
    sprite.loopEnd = frameCount - 1;

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
                        sprite.currentFrame = sprite.loopStart;
                    } else {
                        sprite.currentFrame = sprite.frameCount - 1;
                    }
                } else if (sprite.looping && sprite.currentFrame > sprite.loopEnd) {
                    sprite.currentFrame = sprite.loopStart;
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

    if (!player.grounded) {
        animName = "jump";
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

    // Track previous animation and crouch exit state
    static std::string lastAnim = "idle";
    static bool crouchExiting = false;
    std::string intendedAnim = animName; // Save for after exit

    // Check if crouch exit should start (left crouch state, but not for spindash/roll)
    if (!crouchExiting && lastAnim == "crouch" && animName != "crouch"
        && !player.spindashing && !player.rolling) {
        crouchExiting = true;
        animName = "crouch";
        auto& cs = sprites["crouch"];
        cs.currentFrame = 15;
        cs.frameTimer = 0.0f;
        cs.loopStart = 15;
        cs.loopEnd = 16;
        cs.looping = false;
    }

    bool resetAnim = (animName != lastAnim);
    if (!crouchExiting && resetAnim) {
        lastAnim = animName;
    }

    // On entering crouch normally, set loop range to skip entry/exit frames
    if (!crouchExiting && animName == "crouch" && resetAnim) {
        auto& cs = sprites["crouch"];
        cs.currentFrame = 0;
        cs.frameTimer = 0.0f;
        cs.loopStart = 3;
        cs.loopEnd = 14;
        cs.looping = true;
    }

    SetAnimation(animName, false); // false = don't auto-reset (we set frames manually)
    UpdateAnimation(dt);

    // Post-update: check crouch exit completion
    if (crouchExiting) {
        auto& cs = sprites["crouch"];
        animName = "crouch";
        if (cs.currentFrame >= 16 && cs.frameTimer > 0.0f) {
            crouchExiting = false;
            cs.loopStart = 0;
            cs.loopEnd = cs.frameCount - 1;
            cs.looping = true;
            lastAnim = intendedAnim; // Prevent re-triggering exit
        }
    }

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

// Persistent camera state for smooth SPG-style movement
static float camPosX = 0.0f;
static float camPosY = 0.0f;
static bool camInitialized = false;

static inline float signf(float v) {
    return (v > 0.0f) - (v < 0.0f);
}

void SetCamera(const SonicPhysics::CharacterState& player, bool inputDown, bool inputUp) {
    // Scale factors relative to original 320x224 resolution
    const float hScale = static_cast<float>(SCREEN_WIDTH) / 320.0f;   // ~2.65
    const float vScale = static_cast<float>(SCREEN_HEIGHT) / 224.0f;  // ~2.14

    if (!camInitialized) {
        camPosX = player.x;
        camPosY = player.y;
        camInitialized = true;
    }

    // === HORIZONTAL CAMERA (SPG Border System) ===
    bool moving = false;
    if (player.grounded) {
        moving = std::abs(player.groundSpeed) > 0.5f;
    } else {
        moving = std::abs(player.velocityX) > 0.5f;
    }

    // Target screen X for the player (in original 320px coordinates)
    // Center=160, offset ±16 when moving (symmetric 152/168 border system)
    float targetScreenX_orig = 160.0f;
    if (moving && !player.spindashing && !player.rolling) {
        targetScreenX_orig = 160.0f - 16.0f * player.direction;
    }

    // Convert to our resolution
    float targetScreenX = targetScreenX_orig * hScale;

    // Target camera position: player.x + screenWidth/2 - targetScreenX
    float targetCamX = player.x + SCREEN_WIDTH / 2.0f - targetScreenX;

    // Camera catch-up speed (SPG: 16px/step on 320px hardware limit)
    float maxSpeedX = 16.0f * hScale;
    float diffX = targetCamX - camPosX;
    if (std::abs(diffX) > maxSpeedX) {
        camPosX += signf(diffX) * maxSpeedX;
    } else {
        camPosX = targetCamX;
    }

    // === VERTICAL CAMERA (SPG Focal Point System) ===
    // Default focal point = 96 (slightly above center of 224 → top-biased)
    // Looking up   → focal shifts down 104 (camera moves up → see more above)
    // Looking down → focal shifts up 88   (camera moves down → see more below)
    // Airborne → top border = focal-32, bottom = focal+32
    //
    // 2.5 second delay before look up/down activates (like Sonic games)

    static Uint32 lookStartTime = 0;

    bool wantLookDown = inputDown && player.grounded && std::abs(player.groundSpeed) < 0.5f && !player.spindashing;
    bool wantLookUp   = inputUp && player.grounded;

    if (wantLookDown || wantLookUp) {
        if (lookStartTime == 0) lookStartTime = SDL_GetTicks();
    } else {
        lookStartTime = 0;
    }

    bool lookActive = (lookStartTime != 0 && (SDL_GetTicks() - lookStartTime) >= 2250);

    float defaultFocal_orig = 96.0f;
    float targetFocal_orig = defaultFocal_orig;

    bool crouching = wantLookDown && lookActive;
    bool lookingUp = wantLookUp && lookActive;

    if (lookingUp) {
        // Looking up: focal shifts DOWN by 104 (camera moves up)
        targetFocal_orig = defaultFocal_orig + 104.0f;
    } else if (crouching) {
        // Looking down: focal shifts UP by 88 (camera moves down)
        targetFocal_orig = defaultFocal_orig - 88.0f;
    } else if (!player.grounded) {
        targetFocal_orig = defaultFocal_orig;
    }

    float targetFocal = targetFocal_orig * vScale;
    float targetCamY = player.y + SCREEN_HEIGHT / 2.0f - targetFocal;

    // Vertical catch-up speed
    float maxSpeedY_orig = 6.0f;
    if (lookingUp || crouching) {
        maxSpeedY_orig = 2.0f;
    } else if (player.grounded && std::abs(player.groundSpeed) >= 8.0f) {
        maxSpeedY_orig = 16.0f;
    }
    float maxSpeedY = maxSpeedY_orig * vScale;

    float diffY = targetCamY - camPosY;
    if (std::abs(diffY) > maxSpeedY) {
        camPosY += signf(diffY) * maxSpeedY;
    } else {
        camPosY = targetCamY;
    }

    cameraX = static_cast<int>(camPosX);
    cameraY = static_cast<int>(camPosY);

    // Clamp focus point to world bounds
    int halfScreenW = SCREEN_WIDTH / 2;
    int halfScreenH = SCREEN_HEIGHT / 2;
    cameraX = clamp(cameraX, WORLD_MIN_X + halfScreenW, WORLD_MAX_X - halfScreenW);
    cameraY = clamp(cameraY, WORLD_MIN_Y + halfScreenH, WORLD_MAX_Y - halfScreenH);
}

void RenderGroundIndicator() {
    // ground at y=0 in world space maps to screenY = SCREEN_HEIGHT/2 + 0 - cameraY
    int groundY = SCREEN_HEIGHT / 2 - cameraY;

    if (groundY >= 0 && groundY <= SCREEN_HEIGHT) {
        SDL_SetRenderDrawColor(gRenderer, 80, 80, 80, 255);

        SDL_Rect groundRect = { 0, groundY, SCREEN_WIDTH, SCREEN_HEIGHT - groundY };
        SDL_RenderFillRect(gRenderer, &groundRect);

        SDL_SetRenderDrawColor(gRenderer, 100, 100, 100, 255);
        SDL_RenderDrawLine(gRenderer, 0, groundY, SCREEN_WIDTH, groundY);

        SDL_SetRenderDrawColor(gRenderer, 60, 60, 60, 255);
        int worldLeft = cameraX - SCREEN_WIDTH / 2;
        int worldRight = cameraX + SCREEN_WIDTH / 2;
        for (int wx = (worldLeft / 100) * 100; wx <= worldRight; wx += 100) {
            int sx = wx - cameraX + SCREEN_WIDTH / 2;
            if (sx >= 0 && sx <= SCREEN_WIDTH) {
                SDL_RenderDrawLine(gRenderer, sx, groundY - 5, sx, groundY + 5);
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
