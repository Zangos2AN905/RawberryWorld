#ifndef LEVEL_H
#define LEVEL_H

#include <vector>

struct SlopeSegment {
    float x1, y1;  // Start point
    float x2, y2;  // End point
};

class Level {
public:
    void BuildTestLevel();

    // Returns ground Y and angle (radians) at position x.
    // Angle: 0=flat, negative=uphill right, positive=downhill right.
    // Returns false if x is outside level bounds.
    bool GetGroundInfo(float x, float& outY, float& outAngle) const;

    float GetMinX() const { return minX_; }
    float GetMaxX() const { return maxX_; }

private:
    std::vector<SlopeSegment> segments_;
    float minX_ = 0.0f;
    float maxX_ = 0.0f;
};

#endif
