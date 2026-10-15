#include "element.hpp"

#include "constants.hpp"

#include <algorithm>
#include <cmath>

using core::Rect;
using core::Vec2;

namespace game {

Element makeFence(Vec2 from, Vec2 to)
{
    Element e;
    e.type = ElementType::Fence;
    // characters may put their heads through the gaps in the fence (and therefore are on a lower layer)
    e.layer = 3;

    // distance from the top-left corner to the slat (the same for vertical and horizontal fences)
    const float d = (kFenceWidth - kFenceSlatWidth) / 2.0f;
    e.vertical = std::fabs(to.x - from.x) < std::fabs(to.y - from.y);

    // Collision area: as wide as the slat, from the outer edge of the first pole to the outer
    // edge of the last one. (The Qt version had it offset by kFenceWidth/2 from the graphics.)
    if (e.vertical) {
        const float poleDist = std::fabs(to.y - from.y); // between first and last pole centre
        const float height = poleDist + kFenceWidth;
        const float xStart = from.x;
        const float yStart = std::min(from.y, to.y);
        const Vec2 topLeft{xStart - kFenceWidth / 2.0f, yStart - kFenceWidth / 2.0f};
        e.bounds = {topLeft, {kFenceWidth, height}};
        e.collision = Rect(xStart - kFenceSlatWidth / 2.0f, topLeft.y, kFenceSlatWidth, height);
        e.slat = {topLeft.x + d, yStart, kFenceSlatWidth, poleDist};

        const int segments = std::max(1, static_cast<int>(poleDist / kMinFenceSegmentLength));
        const float step = poleDist / static_cast<float>(segments);
        for (int i = 0; i <= segments; ++i)
            e.poles.push_back({xStart, yStart + step * static_cast<float>(i)});
    } else {
        const float poleDist = std::fabs(to.x - from.x);
        const float width = poleDist + kFenceWidth;
        const float xStart = std::min(from.x, to.x);
        const float yStart = from.y;
        const Vec2 topLeft{xStart - kFenceWidth / 2.0f, yStart - kFenceWidth / 2.0f};
        e.bounds = {topLeft, {width, kFenceWidth}};
        e.collision = Rect(topLeft.x, yStart - kFenceSlatWidth / 2.0f, width, kFenceSlatWidth);
        e.slat = {xStart, topLeft.y + d, poleDist, kFenceSlatWidth};

        const int segments = std::max(1, static_cast<int>(poleDist / kMinFenceSegmentLength));
        const float step = poleDist / static_cast<float>(segments);
        for (int i = 0; i <= segments; ++i)
            e.poles.push_back({xStart + step * static_cast<float>(i), yStart});
    }
    return e;
}

std::optional<Element> makeWater(Vec2 corner1, Vec2 corner2, int style)
{
    if (std::fabs(corner1.x - corner2.x) < kMinWaterSize || std::fabs(corner1.y - corner2.y) < kMinWaterSize)
        return std::nullopt;

    Element e;
    e.type = ElementType::Water;
    e.layer = 1;
    e.style = style;
    e.bounds = Rect::fromCorners({std::min(corner1.x, corner2.x), std::min(corner1.y, corner2.y)},
                                 {std::max(corner1.x, corner2.x), std::max(corner1.y, corner2.y)});
    e.collision = e.bounds;
    return e;
}

Element makeBush(Vec2 topLeft, int style)
{
    Element e;
    e.type = ElementType::Bush;
    e.layer = 1;
    e.style = style;
    e.bounds = {topLeft, kBushImageSize};
    e.collision = Rect(topLeft.x + 8.0f, topLeft.y + 8.0f, 24.0f, 24.0f);
    return e;
}

Element makeTree(Vec2 topLeft, int style)
{
    Element e;
    e.type = ElementType::Tree;
    e.layer = 3;
    e.style = style;
    e.bounds = {topLeft, kTreeImageSize};
    // only the trunk blocks; the crown is drawn above the characters
    e.collision = Rect(topLeft.x + 28.0f, topLeft.y + 28.0f, 15.0f, 15.0f);
    return e;
}

Element makeExit(Vec2 topLeft, int style)
{
    Element e;
    e.type = ElementType::Exit;
    e.layer = 2;
    e.style = style;
    e.bounds = {topLeft, kExitImageSize};
    e.collision = e.bounds;
    return e;
}

} // namespace game
