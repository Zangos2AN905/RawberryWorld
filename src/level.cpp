#include "level.h"
#include <cmath>

void Level::BuildTestLevel() {
    segments_.clear();

    // y increases downward, so:
    //   dy < 0 = uphill (ground rises as x → right, y becomes more negative)
    //   dy > 0 = downhill (ground drops as x → right, y becomes more positive)
    //
    // Slope factor (SPG): 0.125 for walking, so even 20° slopes are climbable.
    //   sin(20°) * 0.125 = 0.043/frame; acceleration = 0.047/frame
    //   Max climbable angle before net deceleration: ~22°

    // Far left flat
    segments_.push_back({-3000.0f, 0.0f, -1200.0f, 0.0f});

    // Long flat run-up — player spawns at x=0, y=0
    segments_.push_back({-1200.0f, 0.0f, 600.0f, 0.0f});

    // Gentle downhill ~4.8° (speed boost into the valley)
    segments_.push_back({600.0f, 0.0f, 900.0f, 25.0f});
    segments_.push_back({900.0f, 25.0f, 1200.0f, 50.0f});

    // Flat valley floor
    segments_.push_back({1200.0f, 50.0f, 1500.0f, 50.0f});

    // Uphill 9.5° back to y=0 — fully climbable
    segments_.push_back({1500.0f, 50.0f, 1800.0f, 0.0f});

    // Flat
    segments_.push_back({1800.0f, 0.0f, 2200.0f, 0.0f});

    // Steep uphill 18° — still climbable (slows slightly)
    segments_.push_back({2200.0f, 0.0f, 2500.0f, -100.0f});

    // Steep downhill 18°
    segments_.push_back({2500.0f, -100.0f, 2800.0f, 0.0f});

    // Far right flat
    segments_.push_back({2800.0f, 0.0f, 3000.0f, 0.0f});

    minX_ = -3000.0f;
    maxX_ = 3000.0f;
}

bool Level::GetGroundInfo(float x, float& outY, float& outAngle) const {
    if (segments_.empty()) return false;

    // Out of bounds — no ground
    if (x < segments_.front().x1 || x >= segments_.back().x2) {
        return false;
    }

    // Linear search to find the segment containing x
    for (const auto& seg : segments_) {
        if (x >= seg.x1 && x < seg.x2) {
            float t = (x - seg.x1) / (seg.x2 - seg.x1);
            outY = seg.y1 + t * (seg.y2 - seg.y1);
            outAngle = std::atan2(seg.y2 - seg.y1, seg.x2 - seg.x1);
            return true;
        }
    }

    return false;  // shouldn't reach here given bounds check
}
