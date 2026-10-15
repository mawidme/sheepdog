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
using game::BonusMode;
using game::LevelStatus;

namespace {

constexpr int kNumLevels = 4;
// click closer than this to the dog: bark instead of walking (Qt: kDogBarkClickThreshold)
constexpr float kDogBarkClickThreshold = 30.0f;
constexpr float kDragThresholdDist = 10.0f;
// the view scrolls to keep the dog this far from the screen edges (Qt: gDogMinDistToBorder)
constexpr float kDogMinDistToBorder = 140.0f;
constexpr float kInfoBarWidth = 60.0f;

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS)
constexpr float kOverviewSeconds = game::kOverviewSecondsMobile;
#else
constexpr float kOverviewSeconds = game::kOverviewSecondsDesktop;
#endif

const char* modeImageName(BonusMode m)
{
    static const char* names[] = {"bonus/drunkMode", "bonus/sheepSleepMode", "bonus/sheepOnSpeedMode",
                                  "bonus/timeBonusMode", "bonus/powerBarkMode"};
    return names[static_cast<int>(m)];
}

const char* boxImageName(BonusMode m)
{
    static const char* names[] = {"bonus/drunkBox", "bonus/sheepSleepBox", "bonus/sheepOnSpeedBox",
                                  "bonus/timeBonusBox", "bonus/powerBarkBox"};
    return names[static_cast<int>(m)];
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
    const auto level = json ? game::parseLevel(*json, &error) : std::nullopt;
    if (!level) {
        SDL_Log("%s: %s", file.c_str(), error.c_str());
        return;
    }
    m_play = std::make_unique<game::LevelPlay>(*level, kOverviewSeconds);
    m_movingDog = m_draggingDog = false;
    m_camera.zoom = 1.0f;
    updateCamera();
}

void Demo::updateCamera()
{
    m_camera.viewportSize = m_renderer.screenSize();
    if (!m_play)
        return;
    const game::World& w = m_play->world();
    const Rect levelRect{{0, 0}, w.size()};

    if (m_play->status() == LevelStatus::Overview) {
        // zoom in on the whole level (Level::overviewZooming()), then jump to the dog
        const float t = m_play->overviewProgress();
        const float zoom = game::kOverviewStartZoom + (1.0f - game::kOverviewStartZoom) * t;
        m_camera.zoom = zoom;
        m_camera.position = levelRect.center() - m_camera.viewportSize / (2.0f * zoom);
        return;
    }

    m_camera.zoom = std::max(m_camera.zoom, m_camera.minZoom);
    if (!w.dogs().empty()) {
        // scroll so the dog keeps a minimum distance to the screen edges (Level::checkDogInView())
        const Vec2 dog = m_camera.worldToScreen(w.dogs()[0].position());
        const float margin =
            std::min(kDogMinDistToBorder, std::min(m_camera.viewportSize.x, m_camera.viewportSize.y) / 3.0f);
        const float left = kInfoBarWidth + margin;
        Vec2 shift;
        if (dog.x < left)
            shift.x = dog.x - left;
        else if (dog.x > m_camera.viewportSize.x - margin)
            shift.x = dog.x - (m_camera.viewportSize.x - margin);
        if (dog.y < margin)
            shift.y = dog.y - margin;
        else if (dog.y > m_camera.viewportSize.y - margin)
            shift.y = dog.y - (m_camera.viewportSize.y - margin);
        m_camera.position += shift / m_camera.zoom;
    }
    m_camera.clampTo(levelRect);
    m_play->setVisibleRect(m_camera.visibleWorldRect());
}

void Demo::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN) {
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
    }
    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        m_camera.zoomAt({event.wheel.mouse_x, event.wheel.mouse_y}, std::pow(1.1f, event.wheel.y));
        return;
    }
    if (!m_play)
        return;

    if (m_play->over()) {
        // any key / click: next level after winning, otherwise try again
        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            if (m_play->status() == LevelStatus::Won)
                loadLevel((m_levelNumber + 1) % kNumLevels);
            else
                loadLevel(m_levelNumber);
        }
        return;
    }
    if (m_play->status() == LevelStatus::Playing)
        handlePlayInput(event);
}

void Demo::handlePlayInput(const SDL_Event& event)
{
    game::World& w = m_play->world();
    if (w.dogs().empty())
        return;
    game::Dog& dog = w.dogs()[0];
    // drunk bonus: keyboard directions are inverted
    const bool drunk = m_play->bonus().active(BonusMode::Drunk);

    switch (event.type) {
    case SDL_EVENT_KEY_DOWN: {
        if (event.key.repeat)
            return;
        const SDL_Keycode key = event.key.key;
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
            if (drunk)
                angle = std::fmod(angle + 180.0f, 360.0f);
            dog.target.reset();
            dog.changeAngle(angle);
            dog.changeSpeed(w.tuning.dogSpeed);
        }
        break;
    }
    case SDL_EVENT_KEY_UP: {
        const SDL_Keycode key = event.key.key;
        if (key == SDLK_UP || key == SDLK_W || key == SDLK_DOWN || key == SDLK_S)
            dog.zeroSpeedY();
        else if (key == SDLK_RIGHT || key == SDLK_D || key == SDLK_LEFT || key == SDLK_A)
            dog.zeroSpeedX();
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        if (event.button.button != SDL_BUTTON_LEFT)
            return;
        m_clickPoint = {event.button.x, event.button.y};
        const Vec2 levelPoint = toLevel(m_clickPoint);
        if (distance(dog.position(), levelPoint) > kDogBarkClickThreshold) {
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
        if (m_draggingDog) {
            dog.changeSpeed(0.0f);
            dog.target.reset();
        }
        m_draggingDog = false;
        break;
    default:
        break;
    }
}

void Demo::tick()
{
    if (!m_play)
        return;
    const bool wasOver = m_play->over();
    m_play->tick();
    for (game::GameEvent e : m_play->world().takeEvents()) {
        // sounds come in chunk 8; log the level outcome until there is text
        if (e == game::GameEvent::LevelWon) {
            const game::LevelResult r = m_play->result();
            SDL_Log("level %d done: score %d + time bonus %d = %d", m_levelNumber, r.score, r.timeBonus, r.total());
        } else if (e == game::GameEvent::TimeUp) {
            SDL_Log("level %d: time is up", m_levelNumber);
        }
    }
    if (!wasOver)
        updateCamera();
}

void Demo::drawBonus()
{
    const game::Bonus& b = m_play->bonus();
    if (b.boxVisible()) {
        m_renderer.drawTexture(m_textures.get("bonus/box"), b.boxRect());
        if (!b.surprise()) {
            const gfx::Texture& icon = m_textures.get(boxImageName(b.mode()));
            m_renderer.drawTexture(icon, {b.boxRect().center() - icon.size() * 0.5f, icon.size()});
        }
    }
    if (b.modeImageVisible()) {
        const gfx::Texture& img = m_textures.get(modeImageName(b.mode()));
        const Vec2 size = img.size() * b.modeImageScale();
        m_renderer.drawTexture(img, {b.modeImageCenter() - size * 0.5f, size}, {.opacity = b.modeImageOpacity()});
    }
}

void Demo::drawInfoBar()
{
    // (Qt: InfoItems) bark power, time with bonus time, one dot per sheep left
    gfx::Renderer& r = m_renderer;
    const game::World& w = m_play->world();
    const Vec2 screen = r.screenSize();
    r.fillRect({0, 0, kInfoBarWidth, screen.y}, Color(255, 255, 255, 100));

    const float power = w.dogs().empty() ? 0.0f : w.dogs()[0].barkPower() / game::kMaxBarkPower;
    const Rect barkBar{8, 20, 18, 200};
    r.fillRect(barkBar, Color(60, 60, 60, 180));
    r.fillRect({barkBar.x, barkBar.bottom() - barkBar.h * power, barkBar.w, barkBar.h * power}, Color(0, 0, 180, 200));

    const game::LevelTime& t = m_play->time();
    const Rect timeBar{34, 20, 18, 200};
    const float total = static_cast<float>(std::max(1, t.total));
    const float left = timeBar.h * static_cast<float>(t.left) / total;
    const float bonus = timeBar.h * static_cast<float>(t.bonus) / total;
    r.fillRect(timeBar, Color(60, 60, 60, 180));
    r.fillRect({timeBar.x, timeBar.bottom() - left, timeBar.w, left}, Color(0, 80, 0, 200));
    r.fillRect({timeBar.x, timeBar.bottom() - left - bonus, timeBar.w, bonus}, Color(9, 150, 9, 200));

    for (int i = 0; i < w.sheepLeft(); ++i)
        r.fillEllipse({22, 240 + 20.0f * static_cast<float>(i), 16, 16}, Color::white());
}

void Demo::draw(float alpha)
{
    gfx::Renderer& r = m_renderer;
    r.clear(Color::black());
    if (!m_play) {
        r.present();
        return;
    }
    // characters only move while playing; don't extrapolate past the last tick otherwise
    if (m_play->status() != LevelStatus::Playing)
        alpha = 1.0f;

    r.setCamera(&m_camera);
    drawWorld(r, m_textures, m_play->world(), alpha, m_showCollisionRects);
    drawBonus();

    r.setCamera(nullptr);
    drawInfoBar();
    if (m_play->over()) {
        const Color tint = m_play->status() == LevelStatus::Won ? Color(0, 160, 0, 110) : Color(160, 0, 0, 110);
        r.fillRect({{0, 0}, r.screenSize()}, tint);
    }
    r.present();
}
