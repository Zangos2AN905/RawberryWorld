#include "physics.h"
#include "level.h"
#include <cmath>
#include <algorithm>

namespace SonicPhysics {

// World bounds
constexpr int WORLD_MIN_X = -3000;
constexpr int WORLD_MAX_X = 3000;
constexpr int WORLD_MIN_Y = -1000;
constexpr int WORLD_MAX_Y = 1000;

inline float clamp(float value, float min, float max) {
    return std::max(min, std::min(max, value));
}

inline float sign(float value) {
    return (value > 0) - (value < 0);
}

PhysicsEngine::PhysicsEngine() = default;

void PhysicsEngine::Init() {
    player_ = CharacterState();
    prevJump_ = false;
}

void PhysicsEngine::Update(bool inputLeft, bool inputRight, bool inputJump, bool inputDown, int deltaTime) {
    const float dt = deltaTime / 1000.0f * 60.0f;

    // Check ground at current position to determine grounded state
    if (level_) {
        float groundY, groundAngle;
        if (level_->GetGroundInfo(player_.x, groundY, groundAngle)) {
            // Within 4px of surface = grounded
            if (player_.y >= groundY - 4.0f) {
                player_.grounded = true;
                player_.angle = groundAngle;
            } else {
                player_.grounded = false;
            }
        } else {
            player_.grounded = false;
        }
    } else {
        // Fallback flat ground at y=0
        if (player_.y >= 0.0f) {
            player_.grounded = true;
            player_.angle = 0.0f;
        }
    }

    if (player_.grounded) {
        UpdateGround(inputLeft, inputRight, inputJump, inputDown, dt);
    } else {
        UpdateAir(inputLeft, inputRight, inputJump, dt);
    }

    // Don't apply velocity movement during spindash charge (player stays put)
    if (!player_.spindashing) {
        player_.x += player_.velocityX * dt;
        player_.y += player_.velocityY * dt;
    }

    // Clamp player position to world bounds
    player_.x = clamp(player_.x, static_cast<float>(WORLD_MIN_X), static_cast<float>(WORLD_MAX_X));
    player_.y = clamp(player_.y, static_cast<float>(WORLD_MIN_Y), static_cast<float>(WORLD_MAX_Y));

    // Post-movement ground collision
    ResolveGroundCollision();

    if (player_.flying) {
        UpdateFlight(inputLeft, inputRight, inputJump, dt);
    }
}

void PhysicsEngine::UpdateGround(bool inputLeft, bool inputRight, bool inputJump, bool inputDown, float dt) {
    // Slope factor (SPG:Slope_Physics)
    // sin(angle) for our atan2 convention:
    //   angle < 0 (uphill right)  → sin < 0 → groundSpeed decreases
    //   angle > 0 (downhill right) → sin > 0 → groundSpeed increases
    float slopeFactor;
    if (player_.rolling) {
        // Rolling: uphill when moving against gravity direction
        if (player_.groundSpeed * std::sin(player_.angle) < 0.0f) {
            slopeFactor = SLOPE_FACTOR_ROLL_UP;
        } else {
            slopeFactor = SLOPE_FACTOR_ROLL_DOWN;
        }
    } else {
        slopeFactor = SLOPE_FACTOR_NORMAL;
    }
    player_.groundSpeed += std::sin(player_.angle) * slopeFactor * dt;

    bool jumpPressed = inputJump && !prevJump_;
    prevJump_ = inputJump;

    // === SPINDASH CHARGING ===
    if (player_.spindashing) {
        // SPG decay: spinrev -= ((spinrev / 0.125) / 256)
        spinrev_ -= (spinrev_ / 0.125f) / 256.0f;
        if (spinrev_ < 0.0f) spinrev_ = 0.0f;

        // Each jump press adds 2 revs (up to 8)
        if (jumpPressed && spinrev_ < SPINDASH_MAX_REVS) {
            spinrev_ += SPINDASH_REVS_PER_PRESS;
            if (spinrev_ > SPINDASH_MAX_REVS)
                spinrev_ = SPINDASH_MAX_REVS;
        }

        // Release Down to launch
        if (!inputDown) {
            ReleaseSpindash();
        } else {
            // Keep player stationary during charge
            player_.velocityX = 0.0f;
            player_.velocityY = 0.0f;
        }
        return;
    }

    // === SPINDASH INITIATION (takes priority over jump when crouching) ===
    if (inputDown && std::abs(player_.groundSpeed) < 0.5f && !player_.rolling && jumpPressed) {
        StartSpindash();
        return;
    }

    // === JUMP (allowed even while rolling) ===
    if (inputJump) {
        DoJump();
        return;
    }

    // === ROLLING STATE ===
    if (player_.rolling) {
        // Rolling: no input acceleration, just friction and slopes
        if (player_.groundSpeed > 0.0f) {
            player_.groundSpeed = std::max(0.0f, player_.groundSpeed - ROLLING_FRICTION * dt);
        } else if (player_.groundSpeed < 0.0f) {
            player_.groundSpeed = std::min(0.0f, player_.groundSpeed + ROLLING_FRICTION * dt);
        }

        // Stop rolling at very low speed
        if (std::abs(player_.groundSpeed) < 0.5f) {
            StopRoll();
        }
    } else {
        // === WALKING STATE ===
        // Input handling with acceleration/friction
        // TOP_SPEED is a soft cap: acceleration stops above it, but
        // external forces (slopes, rolling) can push speed higher.
        float accel = ACCELERATION;
        float decel = DECELERATION;
        float friction = FRICTION;

        if (inputLeft && inputRight) {
            // Both directions: brake to stop
            if (std::abs(player_.groundSpeed) > 0.0f) {
                player_.groundSpeed -= sign(player_.groundSpeed) * decel * dt;
                if (std::abs(player_.groundSpeed) < 0.5f)
                    player_.groundSpeed = 0.0f;
            }
        } else if (inputLeft) {
            if (player_.groundSpeed > 0.0f)
                player_.groundSpeed -= decel * dt;
            else if (player_.groundSpeed > -TOP_SPEED)
                player_.groundSpeed -= accel * dt;
            // else: beyond soft cap leftward, no extra acceleration
            player_.direction = -1;
        } else if (inputRight) {
            if (player_.groundSpeed < 0.0f)
                player_.groundSpeed += decel * dt;
            else if (player_.groundSpeed < TOP_SPEED)
                player_.groundSpeed += accel * dt;
            // else: beyond soft cap rightward, no extra acceleration
            player_.direction = 1;
        } else {
            // No input: friction
            if (player_.groundSpeed > 0.0f)
                player_.groundSpeed = std::max(0.0f, player_.groundSpeed - friction * dt);
            else if (player_.groundSpeed < 0.0f)
                player_.groundSpeed = std::min(0.0f, player_.groundSpeed + friction * dt);
        }
    }

    // Hard cap at MAX_SPEED (applies to both walking and rolling)
    player_.groundSpeed = clamp(player_.groundSpeed, -MAX_SPEED, MAX_SPEED);

    // Convert ground speed to velocity
    player_.velocityX = player_.groundSpeed * std::cos(player_.angle);
    player_.velocityY = player_.groundSpeed * std::sin(player_.angle);

    // Manual roll start (press down while moving)
    if (inputDown && std::abs(player_.groundSpeed) > 1.0f && !player_.rolling) {
        StartRoll();
    }

    // Stand up from roll
    if (!inputDown && player_.rolling && std::abs(player_.groundSpeed) < 0.5f) {
        StopRoll();
    }
}

void PhysicsEngine::UpdateAir(bool inputLeft, bool inputRight, bool inputJump, float dt) {
    // Variable jump height (SPG): cap upward speed if jump button released
    // Check happens BEFORE gravity is applied (per SPG spec)
    if (!inputJump && player_.velocityY < -8.0f) {
        player_.velocityY = -8.0f;
    }

    // Gravity
    player_.velocityY += GRAVITY * dt;

    // Air control
    if (inputLeft) {
        player_.velocityX -= ACCELERATION_AIR * dt;
        player_.direction = -1;
    }
    if (inputRight) {
        player_.velocityX += ACCELERATION_AIR * dt;
        player_.direction = 1;
    }

    // Clamp air speed
    player_.velocityX = clamp(player_.velocityX, -TOP_SPEED * 2.0f, TOP_SPEED * 2.0f);
}

void PhysicsEngine::DoJump() {
    if (!player_.grounded) return;
    // SPG: jump perpendicular to ground angle, preserving existing momentum
    // Our atan2 convention: sin(angle) positive = downhill right, negative = uphill right
    // SPG: X Speed -= jump_force * sin(angle)  →  velocityX += JUMP_FORCE * sin(angle)
    // SPG: Y Speed -= jump_force * cos(angle)  →  velocityY -= JUMP_FORCE * cos(angle)
    player_.velocityX += JUMP_FORCE * std::sin(player_.angle);
    player_.velocityY -= JUMP_FORCE * std::cos(player_.angle);
    player_.grounded = false;
    player_.rolling = false;
}

void PhysicsEngine::StartRoll() {
    player_.rolling = true;
    float speed = std::sqrt(player_.velocityX * player_.velocityX + player_.velocityY * player_.velocityY);
    player_.groundSpeed = speed * player_.direction;
}

void PhysicsEngine::StopRoll() {
    player_.rolling = false;
}

void PhysicsEngine::StartSpindash() {
    player_.spindashing = true;
    spinrev_ = 0.0f;
    player_.rolling = true;
    player_.groundSpeed = 0.0f;
    player_.velocityX = 0.0f;
    player_.velocityY = 0.0f;
}

void PhysicsEngine::ReleaseSpindash() {
    player_.spindashing = false;

    // SPG: Ground Speed = 16 + floor(spinrev / 2), capped at 24
    float releaseSpeed = 16.0f + std::floor(spinrev_ / 2.0f);
    releaseSpeed = std::min(releaseSpeed, 24.0f);

    player_.groundSpeed = releaseSpeed * player_.direction;
    player_.velocityX = player_.groundSpeed * std::cos(player_.angle);
    player_.velocityY = player_.groundSpeed * std::sin(player_.angle);
    spinrev_ = 0.0f;
    // rolling stays true for post-launch roll
}

void PhysicsEngine::UpdateFlight(bool inputLeft, bool inputRight, bool inputJump, float dt) {
    player_.flightTimer--;
    if (player_.flightTimer <= 0) {
        player_.flying = false;
        return;
    }

    player_.velocityY += TAILS_FLIGHT_GRAV * dt;

    if (inputLeft) {
        player_.velocityX -= ACCELERATION_AIR * dt;
    }
    if (inputRight) {
        player_.velocityX += ACCELERATION_AIR * dt;
    }

    player_.velocityX = clamp(player_.velocityX, -TOP_SPEED * 1.5f, TOP_SPEED * 1.5f);
}

void PhysicsEngine::ResolveGroundCollision() {
    if (level_) {
        float groundY, groundAngle;
        if (level_->GetGroundInfo(player_.x, groundY, groundAngle)) {
            if (player_.y >= groundY) {
                // Snap to ground surface
                player_.y = groundY;
                player_.angle = groundAngle;

                // Project world velocity onto the ground surface
                float cosA = std::cos(groundAngle);
                float sinA = std::sin(groundAngle);
                player_.groundSpeed = player_.velocityX * cosA + player_.velocityY * sinA;
                player_.velocityX = player_.groundSpeed * cosA;
                player_.velocityY = player_.groundSpeed * sinA;

                if (!player_.grounded) {
                    player_.grounded = true;
                }
                return;
            }
        }
        // Above ground or out of bounds: grounded state unchanged
        // (will be set correctly on next frame's initial check)
    } else {
        // Fallback flat ground at y=0
        if (player_.y > 0.0f) {
            player_.y = 0.0f;
            player_.velocityY = 0.0f;
            player_.grounded = true;
            player_.angle = 0.0f;
        }
    }
}

} // namespace SonicPhysics
