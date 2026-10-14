#pragma once

#include "core/rect.hpp"
#include "core/vec2.hpp"

#include <algorithm>

namespace gfx {

// Maps world (level) coordinates to screen coordinates: pan + uniform zoom.
// Replaces scaling the level QGraphicsItem and scrolling the QGraphicsView.
struct Camera {
    core::Vec2 position;     // world point shown at the top-left corner of the viewport
    float      zoom = 1.0f;  // screen pixels per world unit
    core::Vec2 viewportSize; // in screen coordinates
    float      minZoom = 0.3f;
    float      maxZoom = 3.0f;

    core::Vec2 worldToScreen(core::Vec2 p) const { return (p - position) * zoom; }
    core::Vec2 screenToWorld(core::Vec2 s) const { return s / zoom + position; }

    core::Rect worldToScreen(const core::Rect& r) const
    {
        return {worldToScreen(r.pos()), r.size() * zoom};
    }

    core::Rect visibleWorldRect() const { return {position, viewportSize / zoom}; }

    // Zoom by factor while keeping the world point under screenPoint fixed (mouse wheel, pinch).
    void zoomAt(core::Vec2 screenPoint, float factor)
    {
        const core::Vec2 anchor = screenToWorld(screenPoint);
        zoom = std::clamp(zoom * factor, minZoom, maxZoom);
        position = anchor - screenPoint / zoom;
    }

    // Pan by a screen-space delta (drag): content follows the finger.
    void panByScreen(core::Vec2 screenDelta) { position -= screenDelta / zoom; }

    // Keep the view inside worldBounds; an axis where the world is smaller than the view is centred.
    void clampTo(const core::Rect& worldBounds)
    {
        const core::Vec2 view = viewportSize / zoom;
        auto clampAxis = [](float pos, float viewLen, float min, float len) {
            if (viewLen >= len)
                return min - (viewLen - len) * 0.5f;
            return std::clamp(pos, min, min + len - viewLen);
        };
        position.x = clampAxis(position.x, view.x, worldBounds.x, worldBounds.w);
        position.y = clampAxis(position.y, view.y, worldBounds.y, worldBounds.h);
    }
};

} // namespace gfx
