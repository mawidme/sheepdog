#include "demo.hpp"

#include "core/math.hpp"
#include "core/strings.hpp"
#include "gfx/renderer.hpp"
#include "gfx/texture_cache.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

using core::Color;
using core::Rect;
using core::Vec2;

namespace {

const Rect kLevelBounds{0, 0, 1600, 960};

// draws a texture at its natural (SVG) size, centred on pos
void drawCentered(gfx::Renderer& r, const gfx::Texture& tex, Vec2 pos, const gfx::SpriteParams& params = {})
{
    r.drawTexture(tex, {pos - tex.size() * 0.5f, tex.size()}, params);
}

} // namespace

Demo::Demo(gfx::Renderer& renderer, gfx::TextureCache& textures) : m_renderer(renderer), m_textures(textures)
{
    m_camera.viewportSize = m_renderer.screenSize();
    // start zoomed out so the level fills the screen
    m_camera.zoom = std::max(m_camera.viewportSize.x / kLevelBounds.w, m_camera.viewportSize.y / kLevelBounds.h);
    m_camera.clampTo(kLevelBounds);
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
    m_camera.clampTo(kLevelBounds);
}

void Demo::draw()
{
    gfx::Renderer& r = m_renderer;
    const float t = static_cast<float>(m_time);

    r.clear(Color::black());

    // --- world space -------------------------------------------------------
    r.setCamera(&m_camera);
    r.fillRect(kLevelBounds, Color(60, 130, 50));

    // water (rounded rect, as in water.hpp)
    r.fillRoundedRect({200, 500, 360, 220}, 20.0f, Color(0, 0, 170));

    // fence: slat + poles, with outlines
    const Rect slat{700, 196, 600, 8};
    r.fillRect(slat, Color(135, 73, 33));
    r.drawRect(slat, Color(76, 42, 20), 1.0f);
    for (float x = 700; x <= 1300; x += 100) {
        r.fillEllipse({x - 8, 192, 16, 16}, Color(110, 53, 18));
        r.drawEllipse({x - 8, 192, 16, 16}, Color(60, 30, 10), 2.0f);
    }

    // trees, bushes, exit
    drawCentered(r, m_textures.get("tree1"), {250, 150});
    drawCentered(r, m_textures.get("tree1"), {420, 110});
    drawCentered(r, m_textures.get("bush1"), {620, 380});
    drawCentered(r, m_textures.get("bush1"), {1450, 820});
    drawCentered(r, m_textures.get("exit1"), {1500, 120});

    // bonus boxes, and a missing image (magenta placeholder)
    drawCentered(r, m_textures.get("bonus/box"), {700, 800});
    drawCentered(r, m_textures.get("bonus/powerBarkBox"), {760, 800});
    drawCentered(r, m_textures.get("does/not/exist"), {820, 800});

    // sheep walking in a circle, rotated to face their direction of travel
    for (int i = 0; i < 6; ++i) {
        const float a = t * 0.6f + static_cast<float>(i) * (2.0f * core::kPi / 6.0f);
        const Vec2 pos = Vec2(1000, 600) + Vec2(std::cos(a), std::sin(a)) * 200.0f;
        const Vec2 velocity(-std::sin(a), std::cos(a));
        // as Character::setRotation(-m_angle - m_rotationOffset), images face down
        drawCentered(r, m_textures.get("sheep01"), pos, {.rotationDeg = -velocity.angleDeg() - 90.0f});
    }

    // dog + bark wave: expanding ring that fades out
    const Vec2 dog{1000, 600};
    // walk animation: dog01..dog07
    const int frame = static_cast<int>(t * 10.0f) % 7 + 1;
    drawCentered(r, m_textures.get(core::format("dog%02d", frame)), dog, {.rotationDeg = 45.0f});
    const float phase = std::fmod(t, 1.5f) / 1.5f;
    const float radius = 20.0f + phase * 140.0f;
    r.drawEllipse({dog - Vec2(radius, radius), {2 * radius, 2 * radius}},
                  Color(255, 255, 255).withAlpha(static_cast<std::uint8_t>(200 * (1.0f - phase))), 6.0f);

    // target line, semi-transparent
    r.drawLine(dog, {300, 300}, Color(255, 255, 0, 120), 3.0f);

    // fading, tinted sprite (bonus mode image)
    drawCentered(r, m_textures.get("bonus/powerBarkMode"), {300, 300},
                 {.opacity = 0.5f + 0.5f * std::sin(t * 2.0f), .tint = Color(255, 180, 57)});

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
