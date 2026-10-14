#include "demo.hpp"

#include "core/strings.hpp"
#include "game/level_data.hpp"
#include "gfx/renderer.hpp"
#include "gfx/texture_cache.hpp"
#include "platform/assets.hpp"
#include "world_view.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

using core::Color;
using core::Rect;
using core::Vec2;

namespace {

constexpr int kNumLevels = 4;
// click closer than this to the dog: bark instead of walking (Qt: kDogBarkClickThreshold)
constexpr float kDogBarkClickThreshold = 30.0f;
constexpr float kDragThresholdDist = 10.0f;
// the view scrolls to keep the dog this far from the screen edges (Qt: gDogMinDistToBorder)
constexpr float kDogMinDistToBorder = 140.0f;

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
    const auto level = json ? game::parseLevel(*json, &error) : std::nullopt;
    if (!level) {
        SDL_Log("%s: %s", file.c_str(), error.c_str());
        return;
    }
    m_world = std::make_unique<game::World>(*level);
    m_movingDog = m_draggingDog = false;

    m_camera.viewportSize = m_renderer.screenSize();
    m_camera.zoom = 1.0f;
    followDog();
}

void Demo::followDog()
{
    m_camera.viewportSize = m_renderer.screenSize();
    if (m_world && !m_world->dogs().empty()) {
        // scroll so the dog keeps a minimum distance to the screen edges (Level::update())
        const Vec2 dog = m_camera.worldToScreen(m_world->dogs()[0].position());
        const float margin =
            std::min(kDogMinDistToBorder, std::min(m_camera.viewportSize.x, m_camera.viewportSize.y) / 3.0f);
        Vec2 shift;
        if (dog.x < margin)
            shift.x = dog.x - margin;
        else if (dog.x > m_camera.viewportSize.x - margin)
            shift.x = dog.x - (m_camera.viewportSize.x - margin);
        if (dog.y < margin)
            shift.y = dog.y - margin;
        else if (dog.y > m_camera.viewportSize.y - margin)
            shift.y = dog.y - (m_camera.viewportSize.y - margin);
        m_camera.position += shift / m_camera.zoom;
    }
    if (m_world)
        m_camera.clampTo({{0, 0}, m_world->size()});
}

void Demo::handleEvent(const SDL_Event& event)
{
    if (!m_world)
        return;
    game::World& w = *m_world;
    game::Dog* dog = w.dogs().empty() ? nullptr : &w.dogs()[0];

    switch (event.type) {
    case SDL_EVENT_KEY_DOWN: {
        const SDL_Keycode key = event.key.key;
        if (key >= SDLK_0 && key < SDLK_0 + kNumLevels) {
            loadLevel(static_cast<int>(key - SDLK_0));
            return;
        }
        if (key == SDLK_R) {
            loadLevel(m_levelNumber);
            return;
        }
        if (key == SDLK_C) {
            m_showCollisionRects = !m_showCollisionRects;
            return;
        }
        if (!dog || event.key.repeat)
            return;
        // keyboard: walk in one of four directions (Qt: MyGraphicsScene::keyPressEvent)
        float angle = -1.0f;
        if (key == SDLK_UP || key == SDLK_W)
            angle = 90.0f;
        else if (key == SDLK_RIGHT || key == SDLK_D)
            angle = 0.0f;
        else if (key == SDLK_DOWN || key == SDLK_S)
            angle = 270.0f;
        else if (key == SDLK_LEFT || key == SDLK_A)
            angle = 180.0f;
        else if (key == SDLK_SPACE)
            w.bark();
        if (angle >= 0.0f) {
            dog->target.reset();
            dog->changeAngle(angle);
            dog->changeSpeed(w.tuning.dogSpeed);
        }
        break;
    }
    case SDL_EVENT_KEY_UP: {
        if (!dog)
            return;
        const SDL_Keycode key = event.key.key;
        if (key == SDLK_UP || key == SDLK_W || key == SDLK_DOWN || key == SDLK_S)
            dog->zeroSpeedY();
        else if (key == SDLK_RIGHT || key == SDLK_D || key == SDLK_LEFT || key == SDLK_A)
            dog->zeroSpeedX();
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        if (!dog || event.button.button != SDL_BUTTON_LEFT)
            return;
        m_clickPoint = {event.button.x, event.button.y};
        const Vec2 levelPoint = toLevel(m_clickPoint);
        if (distance(dog->position(), levelPoint) > kDogBarkClickThreshold) {
            m_movingDog = true;
            w.walkTo(0, levelPoint);
        } else {
            w.bark();
        }
        break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
        const Vec2 p{event.motion.x, event.motion.y};
        if (m_movingDog && distance(m_clickPoint, p) > kDragThresholdDist) {
            m_draggingDog = true;
            w.walkTo(0, toLevel(p));
        }
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event.button.button != SDL_BUTTON_LEFT)
            return;
        m_movingDog = false;
        // after dragging, stop on release instead of walking to the last point
        if (m_draggingDog && dog) {
            dog->changeSpeed(0.0f);
            dog->target.reset();
        }
        m_draggingDog = false;
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        m_camera.zoomAt({event.wheel.mouse_x, event.wheel.mouse_y}, std::pow(1.1f, event.wheel.y));
        break;
    default:
        break;
    }
}

void Demo::tick()
{
    if (!m_world)
        return;
    m_world->tick();
    m_world->takeEvents(); // sounds come in chunk 8
    followDog();
}

void Demo::draw(float alpha)
{
    gfx::Renderer& r = m_renderer;
    r.clear(Color::black());
    if (!m_world) {
        r.present();
        return;
    }
    const game::World& w = *m_world;

    r.setCamera(&m_camera);
    drawWorld(r, m_textures, w, alpha, m_showCollisionRects);

    // --- screen space: minimal info bar (bark power, sheep left) ------------
    r.setCamera(nullptr);
    const Vec2 screen = r.screenSize();
    r.fillRect({0, 0, 60, screen.y}, Color(255, 255, 255, 100));
    const float power = w.dogs().empty() ? 0.0f : w.dogs()[0].barkPower() / game::kMaxBarkPower;
    const Rect bar{16, 20, 28, 200};
    r.fillRect(bar, Color(60, 60, 60, 180));
    r.fillRect({bar.x, bar.bottom() - bar.h * power, bar.w, bar.h * power}, Color(0, 0, 180, 200));
    // one dot per sheep still in the level
    for (int i = 0; i < w.sheepLeft(); ++i)
        r.fillEllipse({22, 240 + 20.0f * static_cast<float>(i), 16, 16}, Color::white());

    r.present();
}
