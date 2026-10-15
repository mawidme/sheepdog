#pragma once

#include "core/vec2.hpp"
#include "game/level_play.hpp"
#include "gfx/camera.hpp"

#include <memory>

union SDL_Event;

namespace gfx {
class Renderer;
class TextureCache;
}

// Plays the built-in levels until the real game flow with menus exists (chunk 7).
// No text yet: the end of a level is shown as a coloured overlay (green: won, red: time up),
// any key or click continues.
//   arrows / WASD: walk, space: bark, click: walk there (on the dog: bark), drag: dog follows
//   wheel: zoom, 0-3: level, R: restart, C: collision areas
class Demo {
public:
    Demo(gfx::Renderer& renderer, gfx::TextureCache& textures);

    void handleEvent(const SDL_Event& event); // event in render coordinates
    void tick();                              // one simulation step (1 / game::kTicksPerSecond)
    void draw(float alpha);                   // alpha: progress towards the next tick

private:
    void loadLevel(int number);
    void updateCamera();
    void handlePlayInput(const SDL_Event& event);
    void drawBonus();
    void drawInfoBar();
    core::Vec2 toLevel(core::Vec2 screen) const { return m_camera.screenToWorld(screen); }

    gfx::Renderer&     m_renderer;
    gfx::TextureCache& m_textures;
    gfx::Camera        m_camera;
    std::unique_ptr<game::LevelPlay> m_play;
    int  m_levelNumber = 0;
    bool m_showCollisionRects = false;

    // mouse / touch control (Qt: MyGraphicsScene)
    bool       m_movingDog = false;
    bool       m_draggingDog = false;
    core::Vec2 m_clickPoint;
};
