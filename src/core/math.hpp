#pragma once

#include <cmath>

namespace core {

inline constexpr float kPi = 3.14159265358979323846f;

constexpr float degToRad(float deg) { return deg * (kPi / 180.0f); }
constexpr float radToDeg(float rad) { return rad * (180.0f / kPi); }

// relative comparison, like Qt's qFuzzyCompare (does not work for values near 0)
inline bool fuzzyEqual(float a, float b)
{
    return std::fabs(a - b) * 100000.0f <= std::fmin(std::fabs(a), std::fabs(b));
}

inline bool fuzzyIsZero(float a) { return std::fabs(a) <= 0.00001f; }

} // namespace core
