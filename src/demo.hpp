#pragma once

#include "gfx/camera.hpp"
#include "gfx/texture.hpp"

union SDL_Event;

namespace gfx { class Renderer; }

// Renderer test scene (chunk 3): exercises every primitive the game needs.
// Drag to pan, mouse wheel to zoom. Replaced by the real game in later chunks.
class Demo {
public:
    explicit Demo(gfx::Renderer& renderer);

    void handleEvent(const SDL_Event& event); // event in render coordinates
    void update(double dt);
    void draw();

private:
    gfx::Renderer& m_renderer;
    gfx::Camera    m_camera;
    gfx::Texture   m_sheep;
    double         m_time = 0.0;
    bool           m_dragging = false;
};
