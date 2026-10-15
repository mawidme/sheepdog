#include "world_view.hpp"

#include "core/strings.hpp"
#include "game/world.hpp"
#include "gfx/renderer.hpp"
#include "gfx/texture_cache.hpp"

using core::Color;
using core::Rect;
using core::Vec2;
using game::Element;
using game::ElementType;

namespace {

// colours of the Qt version (globals.cpp)
constexpr Color kBackgroundColor{0, 100, 0};
constexpr Color kFenceSlatPen{96, 48, 16};
constexpr Color kFenceSlatBrush{135, 73, 33};
constexpr Color kFencePolePen{76, 42, 20};
constexpr Color kFencePoleBrush{110, 53, 18};
constexpr Color kWaterColor{0, 0, 170};
constexpr Color kBarkWaveColor{0, 0, 0, 20};

void drawElement(gfx::Renderer& r, gfx::TextureCache& textures, const Element& e)
{
    switch (e.type) {
    case ElementType::Fence:
        r.fillRect(e.slat, kFenceSlatBrush);
        r.drawRect(e.slat, kFenceSlatPen, 1.0f);
        for (Vec2 p : e.poles) {
            const Rect pole{p - Vec2(game::kFenceWidth, game::kFenceWidth) * 0.5f, {game::kFenceWidth, game::kFenceWidth}};
            r.fillEllipse(pole, kFencePoleBrush);
            r.drawEllipse(pole, kFencePolePen, 1.0f);
        }
        break;
    case ElementType::Water:
        r.fillRoundedRect(e.bounds, game::kMinWaterSize / 2.0f, kWaterColor);
        break;
    case ElementType::Bush:
        r.drawTexture(textures.get(core::format("bush%d", e.style)), e.bounds);
        break;
    case ElementType::Tree:
        r.drawTexture(textures.get(core::format("tree%d", e.style)), e.bounds);
        break;
    case ElementType::Exit:
        r.drawTexture(textures.get(core::format("exit%d", e.style)), e.bounds);
        break;
    }
}

void drawElements(gfx::Renderer& r, gfx::TextureCache& textures, const std::vector<Element>& elements, int minLayer,
                  int maxLayer)
{
    for (const Element& e : elements)
        if (e.layer >= minLayer && e.layer <= maxLayer)
            drawElement(r, textures, e);
}

void drawCharacter(gfx::Renderer& r, const gfx::Texture& tex, const game::Character& c, float alpha)
{
    const Vec2 mid = c.previousPosition() + (c.position() - c.previousPosition()) * alpha;
    const Vec2 topLeft = mid - c.size() * 0.5f;
    r.drawTexture(tex, {topLeft, c.size()},
                  {.rotationDeg = c.rotationDeg(), .pivot = {c.rotationPivot().x / c.size().x, c.rotationPivot().y / c.size().y}});
}

} // namespace

void drawWorld(gfx::Renderer& r, gfx::TextureCache& textures, const game::World& world, float alpha,
               bool showCollisionRects)
{
    r.fillRect({{0, 0}, world.size()}, kBackgroundColor);

    // same stacking as Level::addItemsToScene(): layers 0-1, dogs, sheep, layer 2 (exits), layer 3
    drawElements(r, textures, world.elements(), 0, 1);
    for (const game::Dog& d : world.dogs())
        drawCharacter(r, textures.get(core::format("dog%02d", d.animationStep() + 1)), d, alpha);
    for (const game::Sheep& s : world.sheep())
        if (!s.rescued)
            drawCharacter(r, textures.get("sheep01"), s, alpha);
    drawElements(r, textures, world.elements(), 2, 3);

    // (the Qt version blurred the wave; a faint wide ring looks similar enough for now)
    for (const game::BarkWave& w : world.barkWaves())
        r.drawEllipse({w.center - Vec2(w.radius, w.radius), {2 * w.radius, 2 * w.radius}}, kBarkWaveColor,
                      game::kBarkWaveWidth);

    if (showCollisionRects) {
        const Color col{0xff, 0xb4, 0x39, 100};
        for (const Element& e : world.elements())
            if (e.collision)
                r.fillRect(*e.collision, col);
        for (const game::Dog& d : world.dogs())
            r.fillRect(d.collisionRect(), col);
        for (const game::Sheep& s : world.sheep())
            if (!s.rescued)
                r.fillRect(s.collisionRect(), col);
    }
}

void drawLevelData(gfx::Renderer& r, gfx::TextureCache& textures, const game::LevelData& level)
{
    std::vector<Element> elements;
    for (const game::FenceData& f : level.fences)
        elements.push_back(game::makeFence(f.from, f.to));
    for (const game::WaterData& w : level.water)
        if (auto e = game::makeWater(w.from, w.to, w.style))
            elements.push_back(*e);
    for (const game::PropData& b : level.bushes)
        elements.push_back(game::makeBush(b.pos, b.style));
    for (const game::PropData& t : level.trees)
        elements.push_back(game::makeTree(t.pos, t.style));
    for (const game::PropData& e : level.exits)
        elements.push_back(game::makeExit(e.pos, e.style));

    drawElements(r, textures, elements, 0, 1);
    for (Vec2 d : level.dogs)
        drawCharacter(r, textures.get("dog01"), game::Dog(d), 1.0f);
    // facing down (270 degrees), like the images
    for (Vec2 s : level.sheep)
        drawCharacter(r, textures.get("sheep01"), game::Sheep(s, 0.0f, 270.0f, 0), 1.0f);
    drawElements(r, textures, elements, 2, 3);
}
