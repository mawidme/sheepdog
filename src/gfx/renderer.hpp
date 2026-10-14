#pragma once

#include "core/color.hpp"
#include "core/rect.hpp"
#include "core/vec2.hpp"

#include <vector>

struct SDL_Renderer;
struct SDL_Vertex;

namespace gfx {

class Texture;
struct Camera;

struct SpriteParams {
    float      rotationDeg = 0.0f;  // clockwise on screen, same as QGraphicsItem::setRotation
    core::Vec2 pivot{0.5f, 0.5f};   // rotation origin, relative to the destination rect
    float      opacity = 1.0f;
    core::Color tint = core::Color::white();
    bool       flipX = false;
    const core::Rect* source = nullptr; // sub-rect in logical units (as Texture::size()), nullptr = whole texture
};

// Thin 2D drawing layer over SDL_Renderer.
// All coordinates are world coordinates while a camera is set, screen (logical) coordinates otherwise.
// Stroke widths are in the same units as the coordinates, so lines scale with the zoom like in Qt.
class Renderer {
public:
    explicit Renderer(SDL_Renderer* renderer);

    SDL_Renderer* sdl() const { return m_renderer; }

    // logical screen size (the 2D canvas everything is laid out on)
    core::Vec2 screenSize() const;

    void clear(core::Color color);
    void present();

    // nullptr switches to screen space (UI). The camera must outlive its use.
    void setCamera(const Camera* camera) { m_camera = camera; }

    // clip in screen coordinates, nullptr disables clipping
    void setClipRect(const core::Rect* screenRect);

    void drawTexture(const Texture& texture, const core::Rect& dst, const SpriteParams& params = {});

    void fillRect(const core::Rect& r, core::Color color);
    void drawRect(const core::Rect& r, core::Color color, float width);
    void fillEllipse(const core::Rect& bounds, core::Color color);
    void drawEllipse(const core::Rect& bounds, core::Color color, float width);
    void fillRoundedRect(const core::Rect& r, float radius, core::Color color);
    void drawLine(core::Vec2 a, core::Vec2 b, core::Color color, float width);

private:
    core::Vec2 toScreen(core::Vec2 p) const;
    core::Rect toScreen(const core::Rect& r) const;
    float toScreen(float length) const;

    // draws m_triangles (already in screen space) in one colour, then clears it
    void flushTriangles(core::Color color);

    SDL_Renderer*           m_renderer;
    const Camera*           m_camera = nullptr;
    std::vector<core::Vec2> m_triangles;
    std::vector<SDL_Vertex> m_vertices;
};

} // namespace gfx
