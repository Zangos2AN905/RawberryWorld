// By default, we use Sonic Retro's Physics Guide's constants for Accurate movement.

// Physics constants
const float GRAVITY = 0.21875f;
const float TOP_SPEED = 6.0f;
const float ACCELERATION = 0.046875f;
const float DECELERATION = 0.5f;
const float FRICTION = 0.046875f;
const float JUMP_SPEED = -6.5f;
const float ACCELERATION_AIR = 0.09375f;

// Special physics constants for Characters such as Sonic
const float SPINDASH_MINREV = 4;
const float SPINDASH_MAXREV = 10;
const float SPINDASH_ACCEL = 0.2125f; // increases by 0.2125f when jump key is pressed in spindash state
const float TAILS_FLIGHT_GRAV = 0.125f; // Tails' flight gravity is lower than normal gravity

// Other non-physics constants
const float FLIGHT_TIME = 480.0f; // Tails' flight time in frames (8 seconds, calculated in frames)