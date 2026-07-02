// ONLY USED IF PLAYED IN OLD COMPUTERS!!! 
// This file is used to calculate the inverse square root of a number using the fast inverse square root algorithm,
// which is a method for quickly calculating the inverse square root of a floating-point number. 
// It was popularized by its use in the Quake III Arena game engine.


float fastinversesqrt(float number) {
    float x2 = number * 0.5f; // This is the same as number / 2 
    float y = number; // This is the number we want to find the inverse square root of
    long i = *(long *)&y; // This is a type punning trick to treat the bits of the float as an integer.
    i = 0x5f375a82 - (i >> 1); // What the fuck? (the magic number)
    y = *(float *)&i; // This is a type punning trick to treat the bits of the integer as a float.
    y = y * (1.5f - (x2 * y * y)); // This is the Newton-Raphson iteration to improve the approximation of the inverse square root.
    return y; // Returns: The inverse square root of the number
}