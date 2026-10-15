#pragma once

namespace game {
class World;
struct LevelData;
}
namespace gfx {
class Renderer;
class TextureCache;
}

// Draws a game::World in level coordinates (the renderer's camera must be set).
// alpha in [0, 1]: progress towards the next tick, to interpolate character movement
// between the 24 Hz simulation ticks.
void drawWorld(gfx::Renderer& renderer, gfx::TextureCache& textures, const game::World& world, float alpha,
               bool showCollisionRects = false);

// Draws level data without simulating it (editor): elements and characters standing still.
void drawLevelData(gfx::Renderer& renderer, gfx::TextureCache& textures, const game::LevelData& level);
