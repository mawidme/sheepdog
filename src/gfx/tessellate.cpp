#include "tessellate.hpp"

#include "core/math.hpp"

#include <algorithm>
#include <cmath>

using core::Rect;
using core::Vec2;

namespace gfx::tess {

namespace {

void quad(std::vector<Vec2>& out, Vec2 a, Vec2 b, Vec2 c, Vec2 d)
{
    out.insert(out.end(), {a, b, c, a, c, d});
}

Vec2 ellipsePoint(Vec2 center, Vec2 radius, float angleRad)
{
    return {center.x + radius.x * std::cos(angleRad), center.y + radius.y * std::sin(angleRad)};
}

// triangle fan around `center` over a closed polygon
void fan(std::vector<Vec2>& out, Vec2 center, const std::vector<Vec2>& poly)
{
    for (size_t i = 0; i < poly.size(); ++i)
        out.insert(out.end(), {center, poly[i], poly[(i + 1) % poly.size()]});
}

} // namespace

int circleSegments(float screenRadius)
{
    // roughly one segment per 3 screen pixels of circumference
    const int n = static_cast<int>(std::ceil(2.0f * core::kPi * screenRadius / 3.0f));
    return std::clamp(n, 12, 128);
}

void rect(std::vector<Vec2>& out, const Rect& r)
{
    quad(out, r.topLeft(), {r.right(), r.top()}, r.bottomRight(), {r.left(), r.bottom()});
}

void rectOutline(std::vector<Vec2>& out, const Rect& r, float width)
{
    const float h = width * 0.5f;
    const Rect o = r.inflated(h);
    // top and bottom span the full outer width, left and right fill the gap in between
    rect(out, {o.x, o.y, o.w, width});
    rect(out, {o.x, o.bottom() - width, o.w, width});
    rect(out, {o.x, o.y + width, width, o.h - 2 * width});
    rect(out, {o.right() - width, o.y + width, width, o.h - 2 * width});
}

void ellipse(std::vector<Vec2>& out, const Rect& bounds, int segments)
{
    const Vec2 c = bounds.center();
    const Vec2 rad = bounds.size() * 0.5f;
    std::vector<Vec2> poly;
    poly.reserve(static_cast<size_t>(segments));
    for (int i = 0; i < segments; ++i)
        poly.push_back(ellipsePoint(c, rad, 2.0f * core::kPi * static_cast<float>(i) / static_cast<float>(segments)));
    fan(out, c, poly);
}

void ellipseOutline(std::vector<Vec2>& out, const Rect& bounds, float width, int segments)
{
    const Vec2 c = bounds.center();
    const float h = width * 0.5f;
    const Vec2 outer = bounds.size() * 0.5f + Vec2(h, h);
    const Vec2 inner{std::max(0.0f, bounds.w * 0.5f - h), std::max(0.0f, bounds.h * 0.5f - h)};
    for (int i = 0; i < segments; ++i) {
        const float a0 = 2.0f * core::kPi * static_cast<float>(i) / static_cast<float>(segments);
        const float a1 = 2.0f * core::kPi * static_cast<float>(i + 1) / static_cast<float>(segments);
        quad(out, ellipsePoint(c, outer, a0), ellipsePoint(c, outer, a1), ellipsePoint(c, inner, a1),
             ellipsePoint(c, inner, a0));
    }
}

void roundedRect(std::vector<Vec2>& out, const Rect& r, float radius, int segmentsPerCorner)
{
    radius = std::clamp(radius, 0.0f, std::min(r.w, r.h) * 0.5f);
    if (radius <= 0.0f || segmentsPerCorner < 1) {
        rect(out, r);
        return;
    }
    // corner centres clockwise from top-left, each with its arc's start angle (y down)
    const Vec2 centers[4] = {{r.left() + radius, r.top() + radius},
                             {r.right() - radius, r.top() + radius},
                             {r.right() - radius, r.bottom() - radius},
                             {r.left() + radius, r.bottom() - radius}};
    const float startAngles[4] = {core::kPi, 1.5f * core::kPi, 0.0f, 0.5f * core::kPi};

    std::vector<Vec2> poly;
    poly.reserve(static_cast<size_t>(4 * (segmentsPerCorner + 1)));
    for (int c = 0; c < 4; ++c) {
        for (int i = 0; i <= segmentsPerCorner; ++i) {
            const float a = startAngles[c] + 0.5f * core::kPi * static_cast<float>(i) / static_cast<float>(segmentsPerCorner);
            poly.push_back(ellipsePoint(centers[c], {radius, radius}, a));
        }
    }
    fan(out, r.center(), poly);
}

void line(std::vector<Vec2>& out, Vec2 a, Vec2 b, float width)
{
    const Vec2 d = b - a;
    if (d.isNull())
        return;
    const Vec2 n = Vec2(-d.y, d.x).withLength(width * 0.5f);
    quad(out, a + n, b + n, b - n, a - n);
}

} // namespace gfx::tess
