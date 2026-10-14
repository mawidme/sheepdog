#include "demo.hpp"

#include "core/math.hpp"
#include "core/strings.hpp"
#include "gfx/renderer.hpp"
#include "gfx/texture_cache.hpp"
#include "platform/assets.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

using core::Color;
using core::Rect;
using core::Vec2;

namespace {

constexpr int kNumLevels = 4;
constexpr float kFenceWidth = 6.0f;     // pole diameter (as in the Qt version)
constexpr float kFenceSlatWidth = 3.0f;
constexpr float kFencePoleDist = 25.0f;

// draws a texture at its natural (SVG) size, centred on pos
void drawCentered(gfx::Renderer& r, const gfx::Texture& tex, Vec2 pos, const gfx::SpriteParams& params = {})
{
    r.drawTexture(tex, {pos - tex.size() * 0.5f, tex.size()}, params);
}

} // namespace

Demo::Demo(gfx::Renderer& renderer, gfx::TextureCache& textures) : m_renderer(renderer), m_textures(textures)
{
    loadLevel(0);
}

void Demo::loadLevel(int number)
{
    m_levelNumber = number;
    const std::string file = core::format("levels/level%d.json", number);
    std::string error;
    const auto json = platform::readAssetFile(file);
    m_level = json ? game::parseLevel(*json, &error) : std::nullopt;
    if (json && !m_level)
        SDL_Log("%s: %s", file.c_str(), error.c_str());

    const Rect bounds = levelBounds();
    m_camera.viewportSize = m_renderer.screenSize();
    // start with the whole level width (or height) on screen
    m_camera.zoom = std::clamp(std::min(m_camera.viewportSize.x / bounds.w, m_camera.viewportSize.y / bounds.h),
                               m_camera.minZoom, m_camera.maxZoom);
    m_camera.clampTo(bounds);
}

Rect Demo::levelBounds() const
{
    if (!m_level)
        return {0, 0, 800, 480};
    return {0, 0, static_cast<float>(m_level->width), static_cast<float>(m_level->height)};
}

void Demo::handleEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        m_dragging = event.button.button == SDL_BUTTON_LEFT;
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        m_dragging = false;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        if (m_dragging)
            m_camera.panByScreen({event.motion.xrel, event.motion.yrel});
        break;
    case SDL_EVENT_KEY_DOWN:
        // 0-3: switch level, space: next level
        if (event.key.key >= SDLK_0 && event.key.key < SDLK_0 + kNumLevels)
            loadLevel(static_cast<int>(event.key.key - SDLK_0));
        else if (event.key.key == SDLK_SPACE)
            loadLevel((m_levelNumber + 1) % kNumLevels);
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        m_camera.zoomAt({event.wheel.mouse_x, event.wheel.mouse_y}, std::pow(1.1f, event.wheel.y));
        break;
    default:
        break;
    }
}

void Demo::update(double dt)
{
    m_time += dt;
    m_camera.viewportSize = m_renderer.screenSize();
    m_camera.clampTo(levelBounds());
}

void Demo::draw()
{
    gfx::Renderer& r = m_renderer;
    const float t = static_cast<float>(m_time);

    r.clear(Color::black());

    // --- world space -------------------------------------------------------
    r.setCamera(&m_camera);
    r.fillRect(levelBounds(), Color(60, 130, 50));

    const float phase = std::fmod(t, 1.5f) / 1.5f;
    if (m_level) {
        const game::LevelData& level = *m_level;
        for (const game::WaterData& w : level.water)
            r.fillRoundedRect(Rect::fromCorners(w.from, w.to), 10.0f, Color(0, 0, 170));

        // fences: slat between pole centres, poles along it
        for (const game::FenceData& f : level.fences) {
            r.drawLine(f.from, f.to, Color(135, 73, 33), kFenceSlatWidth);
            const float len = distance(f.from, f.to);
            const int poles = std::max(1, static_cast<int>(std::ceil(len / kFencePoleDist)));
            for (int i = 0; i <= poles; ++i) {
                const Vec2 p = f.from + (f.to - f.from) * (static_cast<float>(i) / static_cast<float>(poles));
                r.fillEllipse({p - Vec2(kFenceWidth, kFenceWidth) * 0.5f, {kFenceWidth, kFenceWidth}},
                              Color(110, 53, 18));
            }
        }

        // props are positioned by their top-left corner
        for (const game::PropData& b : level.bushes)
            r.drawTexture(m_textures.get(core::format("bush%d", b.style)), {b.pos, m_textures.get("bush1").size()});
        for (const game::PropData& e : level.exits)
            r.drawTexture(m_textures.get(core::format("exit%d", e.style)), {e.pos, m_textures.get("exit1").size()});

        // characters are positioned by their centre
        for (Vec2 s : level.sheep)
            drawCentered(r, m_textures.get("sheep01"), s, {.rotationDeg = std::sin(t + s.x) * 20.0f});
        const int frame = static_cast<int>(t * 10.0f) % 7 + 1;
        for (Vec2 d : level.dogs) {
            drawCentered(r, m_textures.get(core::format("dog%02d", frame)), d);
            // bark wave: expanding ring that fades out
            const float radius = 20.0f + phase * 80.0f;
            r.drawEllipse({d - Vec2(radius, radius), {2 * radius, 2 * radius}},
                          Color(255, 255, 255).withAlpha(static_cast<std::uint8_t>(200 * (1.0f - phase))), 4.0f);
        }

        // trees last: they cover characters walking below them
        for (const game::PropData& tr : level.trees)
            r.drawTexture(m_textures.get(core::format("tree%d", tr.style)), {tr.pos, m_textures.get("tree1").size()});
    }

    // --- screen space (UI) -------------------------------------------------
    r.setCamera(nullptr);
    const Vec2 screen = r.screenSize();
    const Rect infoBar{0, 0, 60, screen.y};
    r.fillRect(infoBar, Color(0, 0, 0, 150));
    r.fillRect({16, 20, 28, 200}, Color(60, 60, 60, 180));
    r.fillRect({16, 20 + 200 * (1.0f - phase), 28, 200 * phase}, Color(9, 150, 9, 200));
    const char* buttons[] = {"barkButton", "pauseButton", "retryButton"};
    for (int i = 0; i < 3; ++i) {
        const Rect button{8, screen.y - 52.0f * static_cast<float>(i + 1), 44, 44};
        r.drawTexture(m_textures.get(buttons[i]), button);
    }

    r.present();
}
