#pragma once

#include "core/rect.hpp"
#include "core/vec2.hpp"

#include <vector>

// Turns shapes into triangle lists (3 vertices per triangle, appended to `out`)
// for SDL_RenderGeometry. Pure geometry, no SDL, so it can be unit tested.
// Strokes are centred on the outline, like a QPen.
namespace gfx::tess {

// segment count for a full circle of the given on-screen radius
int circleSegments(float screenRadius);

void rect(std::vector<core::Vec2>& out, const core::Rect& r);
void rectOutline(std::vector<core::Vec2>& out, const core::Rect& r, float width);
void ellipse(std::vector<core::Vec2>& out, const core::Rect& bounds, int segments);
void ellipseOutline(std::vector<core::Vec2>& out, const core::Rect& bounds, float width, int segments);
void roundedRect(std::vector<core::Vec2>& out, const core::Rect& r, float radius, int segmentsPerCorner);
void line(std::vector<core::Vec2>& out, core::Vec2 a, core::Vec2 b, float width);

} // namespace gfx::tess
