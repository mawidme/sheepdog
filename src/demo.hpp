#pragma once

#include "gfx/camera.hpp"

union SDL_Event;

namespace gfx {
class Renderer;
class TextureCache;
}

// Renderer test scene (chunk 3): exercises every primitive the game needs.
// Drag to pan, mouse wheel to zoom. Replaced by the real game in later chunks.
class Demo {
public:
    Demo(gfx::Renderer& renderer, gfx::TextureCache& textures);

    void handleEvent(const SDL_Event& event); // event in render coordinates
    void update(double dt);
    void draw();

private:
    gfx::Renderer& m_renderer;
    gfx::Camera    m_camera;
    gfx::TextureCache& m_textures;
    double         m_time = 0.0;
    bool           m_dragging = false;
};
