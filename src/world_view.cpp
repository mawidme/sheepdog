#include "world_view.hpp"

#include "core/strings.hpp"
#include "game/world.hpp"
#include "gfx/renderer.hpp"
#include "gfx/texture_cache.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

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

// Ring with soft edges, like the Qt version's ring pen with a blur effect: the cross-section of a
// ring of `width` blurred with a Gaussian (sigma = blur / 2), drawn as thin concentric rings.
void drawBlurredRing(gfx::Renderer& r, Vec2 center, float radius, float width, float blur, Color color)
{
    const float sigma = std::max(0.5f, blur / 2.0f);
    const float step = 2.0f;
    const float reach = width / 2.0f + 2.0f * sigma;
    const float scale = 1.0f / (sigma * std::sqrt(2.0f));
    for (float d = -reach + step / 2.0f; d < reach; d += step) {
        // fraction of the blurred ring at distance d from its centre line
        const float coverage =
            0.5f * (std::erf((d + width / 2.0f) * scale) - std::erf((d - width / 2.0f) * scale));
        const auto a = static_cast<std::uint8_t>(std::lround(static_cast<float>(color.a) * coverage));
        const float ringRadius = radius + d;
        if (a == 0 || ringRadius <= step / 2.0f)
            continue;
        r.drawEllipse({center - Vec2(ringRadius, ringRadius), {2 * ringRadius, 2 * ringRadius}}, color.withAlpha(a), step);
    }
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
               bool showCollisionRects, bool drawBackground)
{
    if (drawBackground)
        r.fillRect({{0, 0}, world.size()}, kBackgroundColor);

    // same stacking as Level::addItemsToScene(): layers 0-1, dogs, sheep, layer 2 (exits), layer 3
    drawElements(r, textures, world.elements(), 0, 1);
    for (const game::Dog& d : world.dogs())
        drawCharacter(r, textures.get(core::format("dog%02d", d.animationStep() + 1)), d, alpha);
    for (const game::Sheep& s : world.sheep())
        if (!s.rescued)
            drawCharacter(r, textures.get("sheep01"), s, alpha);
    drawElements(r, textures, world.elements(), 2, 3);

    // faint, blurred ring (Qt: ring pen with a QGraphicsBlurEffect of radius 10)
    for (const game::BarkWave& w : world.barkWaves())
        drawBlurredRing(r, w.center, w.radius, game::kBarkWaveWidth, 10.0f, kBarkWaveColor);

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
