#include "physics.h"
#include <cmath>
#include <algorithm>

namespace SonicPhysics {

// World bounds
constexpr int WORLD_MIN_X = -3000;
constexpr int WORLD_MAX_X = 3000;
constexpr int WORLD_MIN_Y = -1000;
constexpr int WORLD_MAX_Y = 1000;

// Forward declarations
void UpdateGroundPhysics(bool inputLeft, bool inputRight, bool inputJump, bool inputDown, float dt);
void UpdateAirPhysics(bool inputLeft, bool inputRight, bool inputJump, float dt);
void UpdateSpindash(bool inputJump, float dt);
void UpdateFlight(bool inputLeft, bool inputRight, bool inputJump, float dt);
void Jump();
void StartRoll();
void StopRoll();
void StartSpindash();
void ReleaseSpindash();
void StartFlight();

CharacterState player;

inline float clamp(float value, float min, float max) {
    return std::max(min, std::min(max, value));
}

inline float sign(float value) {
    return (value > 0) - (value < 0);
}

void InitPhysics() {
    player = CharacterState();
}

void UpdatePhysics(bool inputLeft, bool inputRight, bool inputJump, bool inputDown, int deltaTime) {
    const float dt = deltaTime / 1000.0f * 60.0f;

    if (player.grounded) {
        UpdateGroundPhysics(inputLeft, inputRight, inputJump, inputDown, dt);
    } else {
        UpdateAirPhysics(inputLeft, inputRight, inputJump, dt);
    }

    player.x += player.velocityX * dt;
    player.y += player.velocityY * dt;

    // Clamp player position to world bounds
    player.x = clamp(player.x, static_cast<float>(WORLD_MIN_X), static_cast<float>(WORLD_MAX_X));
    player.y = clamp(player.y, static_cast<float>(WORLD_MIN_Y), static_cast<float>(WORLD_MAX_Y));

    if (player.y > 0.0f) {
        player.y = 0.0f;
        player.velocityY = 0.0f;
        player.grounded = true;
        player.angle = 0.0f;
    }

    if (player.spindashing) {
        UpdateSpindash(inputJump, dt);
    }

    if (player.flying) {
        UpdateFlight(inputLeft, inputRight, inputJump, dt);
    }
}

void UpdateGroundPhysics(bool inputLeft, bool inputRight, bool inputJump, bool inputDown, float dt) {
    float accel = ACCELERATION;
    float decel = DECELERATION;
    float friction = FRICTION;
    float topSpeed = TOP_SPEED;

    if (player.rolling) {
        accel = 0.0f;
        decel = 0.0f;
        friction = 0.0f;
        topSpeed = 16.0f;
    }

    if (inputLeft && inputRight) {
        if (std::abs(player.groundSpeed) > 0.0f) {
            player.groundSpeed -= sign(player.groundSpeed) * decel * dt;
            player.groundSpeed = clamp(player.groundSpeed, -0.5f, 0.5f);
            if (std::abs(player.groundSpeed) < 0.05f) player.groundSpeed = 0.0f;
        }
    } else if (inputLeft) {
        if (player.groundSpeed > 0.0f) {
            player.groundSpeed -= decel * dt;
        } else {
            player.groundSpeed -= accel * dt;
        }
        player.direction = -1;
    } else if (inputRight) {
        if (player.groundSpeed < 0.0f) {
            player.groundSpeed += decel * dt;
        } else {
            player.groundSpeed += accel * dt;
        }
        player.direction = 1;
    } else {
        if (player.groundSpeed > 0.0f) {
            player.groundSpeed = std::max(0.0f, player.groundSpeed - friction * dt);
        } else if (player.groundSpeed < 0.0f) {
            player.groundSpeed = std::min(0.0f, player.groundSpeed + friction * dt);
        }
    }

    player.groundSpeed = clamp(player.groundSpeed, -topSpeed, topSpeed);

    player.velocityX = player.groundSpeed * std::cos(player.angle);
    player.velocityY = player.groundSpeed * std::sin(player.angle);

    if (inputJump && !player.rolling && player.grounded) {
        Jump();
    }

    if (inputDown && std::abs(player.groundSpeed) > 0.0f && !player.rolling) {
        StartRoll();
    } else if (!inputDown && player.rolling && std::abs(player.groundSpeed) < 0.5f) {
        StopRoll();
    }

    if (inputDown && player.groundSpeed == 0.0f && !player.spindashing) {
        StartSpindash();
    }
}

void UpdateAirPhysics(bool inputLeft, bool inputRight, bool inputJump, float dt) {
    player.velocityY += GRAVITY * dt;

    if (inputLeft) {
        player.velocityX -= ACCELERATION_AIR * dt;
        player.direction = -1;
    }
    if (inputRight) {
        player.velocityX += ACCELERATION_AIR * dt;
        player.direction = 1;
    }

    float maxAirSpeed = TOP_SPEED * 2.0f;
    player.velocityX = clamp(player.velocityX, -maxAirSpeed, maxAirSpeed);

    if (inputJump && player.velocityY < 0.0f && player.jumpTimer > 0) {
        player.velocityY -= 0.05f * dt;
        player.jumpTimer--;
    } else {
        player.jumpTimer = 0;
    }
}

void Jump() {
    if (player.grounded) {
        player.velocityY = JUMP_SPEED;
        player.grounded = false;
        player.jumpTimer = 10;
        player.rolling = false;
    }
}

void StartRoll() {
    player.rolling = true;
    float speed = std::sqrt(player.velocityX * player.velocityX + player.velocityY * player.velocityY);
    player.groundSpeed = speed * player.direction;
}

void StopRoll() {
    player.rolling = false;
}

void StartSpindash() {
    player.spindashing = true;
    player.spindashRev = 0;
    player.spindashSpeed = 0.0f;
    player.rolling = true;
    player.groundSpeed = 0.0f;
}

void UpdateSpindash(bool inputJump, float dt) {
    if (inputJump && player.spindashRev < SPINDASH_MAXREV) {
        player.spindashRev++;
        player.spindashSpeed += SPINDASH_ACCEL * dt;
    }

    if (!inputJump || player.spindashRev >= SPINDASH_MAXREV) {
        ReleaseSpindash();
    }
}

void ReleaseSpindash() {
    player.spindashing = false;
    if (player.spindashRev >= SPINDASH_MINREV) {
        player.groundSpeed = player.spindashSpeed * 8.0f * player.direction;
        player.velocityX = player.groundSpeed * std::cos(player.angle);
        player.velocityY = player.groundSpeed * std::sin(player.angle);
    }
    player.spindashRev = 0;
    player.spindashSpeed = 0.0f;
}

void StartFlight() {
    if (!player.flying && !player.grounded) {
        player.flying = true;
        player.flightTimer = static_cast<int>(FLIGHT_TIME);
    }
}

void UpdateFlight(bool inputLeft, bool inputRight, bool inputJump, float dt) {
    player.flightTimer--;
    if (player.flightTimer <= 0 || !inputJump) {
        player.flying = false;
        return;
    }

    player.velocityY += TAILS_FLIGHT_GRAV * dt;

    if (inputLeft) {
        player.velocityX -= ACCELERATION_AIR * dt;
    }
    if (inputRight) {
        player.velocityX += ACCELERATION_AIR * dt;
    }

    float maxFlightSpeed = TOP_SPEED * 1.5f;
    player.velocityX = clamp(player.velocityX, -maxFlightSpeed, maxFlightSpeed);
}

void SetGroundAngle(float newAngle) {
    player.angle = newAngle;
}

void SetGrounded(bool isGrounded) {
    player.grounded = isGrounded;
}

const CharacterState& GetPlayerState() {
    return player;
}

void SetPlayerPosition(float x, float y) {
    player.x = x;
    player.y = y;
}

void SetPlayerVelocity(float vx, float vy) {
    player.velocityX = vx;
    player.velocityY = vy;
}

} // namespace SonicPhysics