#pragma once

#include "vec2.hpp"

#include <algorithm>

namespace core {

// Axis-aligned rectangle, top-left origin. Replaces QRectF.
// Semantics follow QRectF for normalized rects (non-negative size), which is all the game uses.
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    constexpr Rect() = default;
    constexpr Rect(float x_, float y_, float w_, float h_) : x(x_), y(y_), w(w_), h(h_) {}
    constexpr Rect(Vec2 pos, Vec2 size) : x(pos.x), y(pos.y), w(size.x), h(size.y) {}

    static constexpr Rect fromCorners(Vec2 topLeft, Vec2 bottomRight)
    {
        return {topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y};
    }

    constexpr bool operator==(const Rect&) const = default;

    constexpr float left() const { return x; }
    constexpr float top() const { return y; }
    constexpr float right() const { return x + w; }
    constexpr float bottom() const { return y + h; }

    constexpr Vec2 pos() const { return {x, y}; }
    constexpr Vec2 size() const { return {w, h}; }
    constexpr Vec2 topLeft() const { return {x, y}; }
    constexpr Vec2 bottomRight() const { return {x + w, y + h}; }
    constexpr Vec2 center() const { return {x + w * 0.5f, y + h * 0.5f}; }

    constexpr bool isEmpty() const { return w <= 0.0f || h <= 0.0f; }
    constexpr bool isNull() const { return w == 0.0f && h == 0.0f; }

    // edges inclusive, as QRectF::contains(QPointF)
    constexpr bool contains(Vec2 p) const
    {
        return !isNull() && p.x >= x && p.x <= right() && p.y >= y && p.y <= bottom();
    }

    constexpr bool contains(const Rect& r) const
    {
        return !isNull() && !r.isNull() && r.x >= x && r.right() <= right() && r.y >= y &&
               r.bottom() <= bottom();
    }

    // rects that only touch at an edge do not intersect, as QRectF::intersects
    constexpr bool intersects(const Rect& r) const
    {
        return !isNull() && !r.isNull() && x < r.right() && r.x < right() && y < r.bottom() &&
               r.y < bottom();
    }

    // empty Rect() if there is no overlap
    constexpr Rect intersected(const Rect& r) const
    {
        if (!intersects(r))
            return {};
        const float l = std::max(x, r.x);
        const float t = std::max(y, r.y);
        return {l, t, std::min(right(), r.right()) - l, std::min(bottom(), r.bottom()) - t};
    }

    constexpr Rect united(const Rect& r) const
    {
        if (isNull())
            return r;
        if (r.isNull())
            return *this;
        const float l = std::min(x, r.x);
        const float t = std::min(y, r.y);
        return {l, t, std::max(right(), r.right()) - l, std::max(bottom(), r.bottom()) - t};
    }

    constexpr Rect translated(Vec2 d) const { return {x + d.x, y + d.y, w, h}; }
    constexpr Rect translated(float dx, float dy) const { return {x + dx, y + dy, w, h}; }
    constexpr void translate(Vec2 d) { x += d.x; y += d.y; }
    constexpr void translate(float dx, float dy) { x += dx; y += dy; }
    constexpr void moveTo(Vec2 p) { x = p.x; y = p.y; }
    constexpr void moveTo(float px, float py) { x = px; y = py; }
    constexpr void moveCenter(Vec2 c) { x = c.x - w * 0.5f; y = c.y - h * 0.5f; }

    // grow (positive) or shrink (negative) on all sides
    constexpr Rect inflated(float d) const { return {x - d, y - d, w + 2 * d, h + 2 * d}; }
};

} // namespace core
