#ifndef PHYSICS_CONSTANTS_H
#define PHYSICS_CONSTANTS_H

class Level;

namespace SonicPhysics {
    // Physics constants
    constexpr float GRAVITY = 0.4375f;          // ×2 for 848x480
    constexpr float TOP_SPEED = 12.0f;          // Soft cap — input acceleration stops here
    constexpr float MAX_SPEED = 48.0f;          // Hard cap — absolute speed limit
    constexpr float ACCELERATION = 0.09375f;
    constexpr float DECELERATION = 1.0f;
    constexpr float FRICTION = 0.09375f;
    constexpr float ROLLING_FRICTION = 0.046875f;
    constexpr float JUMP_FORCE = 13.0f;
    constexpr float ACCELERATION_AIR = 0.1875f;

    // Spindash constants (SPG spec)
    constexpr float SPINDASH_REVS_PER_PRESS = 2.0f;
    constexpr float SPINDASH_MAX_REVS = 8.0f;

    // Slope factor constants (SPG:Slope_Physics)
    // Applied as: gsp += sin(angle) * SLOPE_FACTOR * dt  (our angle convention)
    constexpr float SLOPE_FACTOR_NORMAL = 0.25f;       // Walking/running
    constexpr float SLOPE_FACTOR_ROLL_UP = 0.15625f;   // Rolling uphill
    constexpr float SLOPE_FACTOR_ROLL_DOWN = 0.625f;   // Rolling downhill

    // Tails flight
    constexpr float TAILS_FLIGHT_GRAV = 0.25f;
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
        void SetLevel(const Level* level) { level_ = level; }

    private:
        void UpdateGround(bool inputLeft, bool inputRight, bool inputJump, bool inputDown, float dt);
        void UpdateAir(bool inputLeft, bool inputRight, bool inputJump, float dt);
        void UpdateFlight(bool inputLeft, bool inputRight, bool inputJump, float dt);
        void DoJump();
        void StartRoll();
        void StopRoll();
        void StartSpindash();
        void ReleaseSpindash();
        void ResolveGroundCollision();

        CharacterState player_;
        bool prevJump_ = false;
        float spinrev_ = 0.0f;  // Spindash revs (internal float for SPG decay)
        const Level* level_ = nullptr;
    };
}

#endif
