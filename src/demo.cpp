#include "demo.hpp"

#include "core/math.hpp"
#include "gfx/renderer.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

using core::Color;
using core::Rect;
using core::Vec2;

namespace {

const Rect kLevelBounds{0, 0, 1600, 960};

// Top-down "sheep" generated in code until the baked PNGs exist (chunk 4).
// Head on the right, so rotation is visible.
gfx::Texture makeSheepTexture(SDL_Renderer* renderer)
{
    constexpr int kSize = 64;
    SDL_Surface* s = SDL_CreateSurface(kSize, kSize, SDL_PIXELFORMAT_RGBA32);
    if (!s)
        return {};
    for (int y = 0; y < kSize; ++y) {
        for (int x = 0; x < kSize; ++x) {
            const float dx = (x + 0.5f - 28.0f) / 24.0f, dy = (y + 0.5f - 32.0f) / 18.0f;
            const float hx = (x + 0.5f - 52.0f) / 9.0f, hy = (y + 0.5f - 32.0f) / 8.0f;
            if (hx * hx + hy * hy <= 1.0f)
                SDL_WriteSurfacePixel(s, x, y, 40, 40, 40, 255);
            else if (dx * dx + dy * dy <= 1.0f)
                SDL_WriteSurfacePixel(s, x, y, 245, 245, 235, 255);
            else
                SDL_WriteSurfacePixel(s, x, y, 0, 0, 0, 0);
        }
    }
    gfx::Texture tex = gfx::Texture::fromSurface(renderer, s);
    SDL_DestroySurface(s);
    return tex;
}

} // namespace

Demo::Demo(gfx::Renderer& renderer) : m_renderer(renderer), m_sheep(makeSheepTexture(renderer.sdl()))
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

    // sheep walking in a circle, rotated to face their direction of travel
    for (int i = 0; i < 6; ++i) {
        const float a = t * 0.6f + static_cast<float>(i) * (2.0f * core::kPi / 6.0f);
        const Vec2 pos = Vec2(1000, 600) + Vec2(std::cos(a), std::sin(a)) * 200.0f;
        const Vec2 velocity(-std::sin(a), std::cos(a));
        // Character::setRotation(-angle): our texture faces right (0 degrees)
        const float rotation = -velocity.angleDeg();
        r.drawTexture(m_sheep, {pos - Vec2(24, 24), {48, 48}}, {.rotationDeg = rotation});
    }

    // dog + bark wave: expanding ring that fades out
    const Vec2 dog{1000, 600};
    r.fillEllipse({dog - Vec2(14, 14), {28, 28}}, Color(150, 90, 40));
    const float phase = std::fmod(t, 1.5f) / 1.5f;
    const float radius = 20.0f + phase * 140.0f;
    r.drawEllipse({dog - Vec2(radius, radius), {2 * radius, 2 * radius}},
                  Color(255, 255, 255).withAlpha(static_cast<std::uint8_t>(200 * (1.0f - phase))), 6.0f);

    // target line, semi-transparent
    r.drawLine(dog, {300, 300}, Color(255, 255, 0, 120), 3.0f);

    // translucent sprite (bonus mode image fading out)
    r.drawTexture(m_sheep, {300, 260, 80, 80},
                  {.opacity = 0.5f + 0.5f * std::sin(t * 2.0f), .tint = Color(255, 180, 57)});

    // --- screen space (UI) -------------------------------------------------
    r.setCamera(nullptr);
    const Vec2 screen = r.screenSize();
    const Rect infoBar{0, 0, 60, screen.y};
    r.fillRect(infoBar, Color(0, 0, 0, 150));
    r.fillRect({16, 20, 28, 200}, Color(60, 60, 60, 180));
    r.fillRect({16, 20 + 200 * (1.0f - phase), 28, 200 * phase}, Color(9, 150, 9, 200));
    for (int i = 0; i < 3; ++i) {
        const Rect button{10, screen.y - 60.0f * static_cast<float>(i + 1), 40, 40};
        r.fillRoundedRect(button, 8.0f, Color(227, 224, 61, 200));
        r.drawRect(button, Color::black(), 2.0f);
    }

    r.present();
}
