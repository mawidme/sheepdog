#pragma once

#include "math.hpp"

#include <cmath>

namespace core {

// 2D point / vector in screen coordinates (y grows downwards).
// Replaces QPointF, and QLineF where it was used as a vector from the origin.
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vec2() = default;
    constexpr Vec2(float x_, float y_) : x(x_), y(y_) {}

    // vector of the given length pointing at angleDeg (same convention as angleDeg())
    static Vec2 fromPolarDeg(float angleDeg, float length)
    {
        const float rad = degToRad(angleDeg);
        return {std::cos(rad) * length, -std::sin(rad) * length};
    }

    constexpr Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2 operator-() const { return {-x, -y}; }
    constexpr Vec2 operator*(float s) const { return {x * s, y * s}; }
    constexpr Vec2 operator/(float s) const { return {x / s, y / s}; }
    constexpr Vec2& operator+=(Vec2 o) { x += o.x; y += o.y; return *this; }
    constexpr Vec2& operator-=(Vec2 o) { x -= o.x; y -= o.y; return *this; }
    constexpr Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
    constexpr Vec2& operator/=(float s) { x /= s; y /= s; return *this; }
    constexpr bool operator==(const Vec2&) const = default;

    constexpr float dot(Vec2 o) const { return x * o.x + y * o.y; }
    constexpr float lengthSquared() const { return x * x + y * y; }
    float length() const { return std::sqrt(lengthSquared()); }
    constexpr bool isNull() const { return x == 0.0f && y == 0.0f; }

    // Same direction, new length. A zero vector stays zero (as QLineF::setLength).
    Vec2 withLength(float newLength) const
    {
        const float len = length();
        return len > 0.0f ? *this * (newLength / len) : *this;
    }

    Vec2 normalized() const { return withLength(1.0f); }

    // Angle in degrees in [0, 360), counter-clockwise on screen, 0 = pointing right.
    // Matches QLineF::angle(), which the game logic is written against.
    float angleDeg() const
    {
        if (isNull())
            return 0.0f;
        float deg = radToDeg(std::atan2(-y, x));
        if (deg < 0.0f)
            deg += 360.0f;
        return deg >= 360.0f ? 0.0f : deg;
    }

    // Same length, new direction (as QLineF::setAngle).
    Vec2 withAngleDeg(float deg) const { return fromPolarDeg(deg, length()); }
};

constexpr Vec2 operator*(float s, Vec2 v) { return v * s; }

inline float distance(Vec2 a, Vec2 b) { return (b - a).length(); }

} // namespace core
