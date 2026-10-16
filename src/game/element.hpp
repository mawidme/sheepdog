#pragma once

#include "core/rect.hpp"
#include "core/vec2.hpp"

#include <optional>
#include <vector>

// Static level elements (fences, water, bushes, trees, exits). Replaces LevelElement and
// its subclasses; geometry is computed exactly as in the Qt version.
namespace game {

enum class ElementType { Fence, Water, Bush, Tree, Exit };

struct Element {
    ElementType type = ElementType::Fence;
    // draw order: 0-1 below the characters, 2-3 above (fences and trees cover characters' heads)
    int layer = 0;
    int style = 1;                       // image variant, e.g. bush1
    core::Rect bounds;                   // area covered by the graphics
    std::optional<core::Rect> collision; // nullopt: characters walk through

    // fences only
    bool vertical = false;
    core::Rect slat;
    std::vector<core::Vec2> poles;       // pole centres
};

// arguments are the centres of the first and the last pole
Element makeFence(core::Vec2 from, core::Vec2 to);
// nullopt if smaller than kMinWaterSize in either direction
std::optional<Element> makeWater(core::Vec2 corner1, core::Vec2 corner2, int style);
// positioned by their top-left corner
Element makeBush(core::Vec2 topLeft, int style);
Element makeTree(core::Vec2 topLeft, int style);
Element makeExit(core::Vec2 topLeft, int style);

} // namespace game
