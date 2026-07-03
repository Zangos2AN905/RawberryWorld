#ifndef PHYSICS_CONSTANTS_H
#define PHYSICS_CONSTANTS_H

namespace SonicPhysics {
    // Physics constants
    constexpr float GRAVITY = 0.21875f;
    constexpr float TOP_SPEED = 6.0f;
    constexpr float ACCELERATION = 0.046875f;
    constexpr float DECELERATION = 0.5f;
    constexpr float FRICTION = 0.046875f;
    constexpr float JUMP_SPEED = -6.5f;
    constexpr float ACCELERATION_AIR = 0.09375f;

    // Special physics constants for Characters such as Sonic
    constexpr float SPINDASH_MINREV = 4;
    constexpr float SPINDASH_MAXREV = 10;
    constexpr float SPINDASH_ACCEL = 0.2125f; // increases by 0.2125f when jump key is pressed in spindash state
    constexpr float TAILS_FLIGHT_GRAV = 0.125f; // Tails' flight gravity is lower than normal gravity

    // Other non-physics constants
    constexpr float FLIGHT_TIME = 480.0f; // Tails' flight time in frames (8 seconds, calculated in frames)

    struct CharacterState {
        float x = 0.0f;
        float y = 0.0f;
        float velocityX = 0.0f;
        float velocityY = 0.0f;
        float groundSpeed = 0.0f;
        float angle = 0.0f;
        bool grounded = false;
        bool rolling = false;
        bool spindashing = false;
        int spindashRev = 0;
        float spindashSpeed = 0.0f;
        int jumpTimer = 0;
        int flightTimer = 0;
        bool flying = false;
        int direction = 1; // 1 = right, -1 = left
    };

    void InitPhysics();
    void UpdatePhysics(bool inputLeft, bool inputRight, bool inputJump, bool inputDown, int deltaTime);
    void SetGroundAngle(float newAngle);
    void SetGrounded(bool isGrounded);
    const CharacterState& GetPlayerState();
    void SetPlayerPosition(float x, float y);
    void SetPlayerVelocity(float vx, float vy);
    void StartFlight();
}

#endif