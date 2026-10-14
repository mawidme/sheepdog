#pragma once

#include "core/rect.hpp"
#include "game/level_data.hpp"
#include "gfx/camera.hpp"

#include <optional>

union SDL_Event;

namespace gfx {
class Renderer;
class TextureCache;
}

// Test scene: draws a built-in level from its JSON file with the renderer.
// Drag to pan, mouse wheel to zoom, keys 0-3 / space to switch levels.
// Replaced by the real game in later chunks.
class Demo {
public:
    Demo(gfx::Renderer& renderer, gfx::TextureCache& textures);

    void handleEvent(const SDL_Event& event); // event in render coordinates
    void update(double dt);
    void draw();

private:
    void loadLevel(int number);
    core::Rect levelBounds() const;

    gfx::Renderer& m_renderer;
    gfx::Camera    m_camera;
    gfx::TextureCache& m_textures;
    std::optional<game::LevelData> m_level;
    int            m_levelNumber = 0;
    double         m_time = 0.0;
    bool           m_dragging = false;
};
