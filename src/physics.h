#ifndef PHYSICS_CONSTANTS_H
#define PHYSICS_CONSTANTS_H

namespace SonicPhysics {
    // Physics constants
    constexpr float GRAVITY = 0.21875f;
    constexpr float TOP_SPEED = 6.0f;
    constexpr float ACCELERATION = 0.046875f;
    constexpr float DECELERATION = 0.5f;
    constexpr float FRICTION = 0.046875f;
    constexpr float ROLLING_FRICTION = 0.0234375f;
    constexpr float JUMP_SPEED = -6.5f;
    constexpr float ACCELERATION_AIR = 0.09375f;

    // Spindash constants (SPG spec)
    constexpr float SPINDASH_REVS_PER_PRESS = 2.0f;
    constexpr float SPINDASH_MAX_REVS = 8.0f;

    // Tails flight
    constexpr float TAILS_FLIGHT_GRAV = 0.125f;
    constexpr float FLIGHT_TIME = 480.0f;

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
        int jumpTimer = 0;
        int flightTimer = 0;
        bool flying = false;
        int direction = 1;
    };

    class PhysicsEngine {
    public:
        PhysicsEngine();

        void Init();
        void Update(bool inputLeft, bool inputRight, bool inputJump, bool inputDown, int deltaTime);

        const CharacterState& GetPlayerState() const { return player_; }

        void SetPlayerPosition(float x, float y) { player_.x = x; player_.y = y; }
        void SetGrounded(bool grounded) { player_.grounded = grounded; }
        void SetPlayerVelocity(float vx, float vy) { player_.velocityX = vx; player_.velocityY = vy; }
        void SetGroundAngle(float angle) { player_.angle = angle; }

    private:
        void UpdateGround(bool inputLeft, bool inputRight, bool inputJump, bool inputDown, float dt);
        void UpdateAir(bool inputLeft, bool inputRight, bool inputJump, float dt);
        void UpdateFlight(bool inputLeft, bool inputRight, bool inputJump, float dt);
        void DoJump();
        void StartRoll();
        void StopRoll();
        void StartSpindash();
        void ReleaseSpindash();

        CharacterState player_;
        bool prevJump_ = false;
        float spinrev_ = 0.0f;  // Spindash revs (internal float for SPG decay)
    };
}

#endif
