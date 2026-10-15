#include "game_app.hpp"

#include "core/strings.hpp"
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

// click closer than this to the dog: bark instead of walking (Qt: kDogBarkClickThreshold)
constexpr float kDogBarkClickThreshold = 30.0f;
constexpr float kDragThresholdDist = 10.0f;
// the view scrolls to keep the dog this far from the screen edges (Qt: gDogMinDistToBorder)
constexpr float kDogMinDistToBorder = 140.0f;
constexpr Color kBackgroundColor{0, 100, 0};

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

bool isTouchMouse(const SDL_Event& e)
{
    switch (e.type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        return e.button.which == SDL_TOUCH_MOUSEID;
    case SDL_EVENT_MOUSE_MOTION:
        return e.motion.which == SDL_TOUCH_MOUSEID;
    default:
        return false;
    }
}

std::string userLevelPath(int number)
{
    return platform::userDataPath() + core::format("levels/level%d.json", number);
}

} // namespace

GameApp::GameApp(gfx::Renderer& renderer, gfx::TextureCache& textures)
    : m_renderer(renderer), m_textures(textures), m_font(gfx::Font::load(renderer.sdl(), textures.scale()))
{
    showMenu();
}

// =============================================================================================
// flow (Qt: goToGameMode(), restorePrevGameMode(), Level::menu(), Level::finish(), ...)

int GameApp::levelCount(LevelSource source) const
{
    int n = 0;
    while (true) {
        const int number = n + 1;
        const bool exists = source == LevelSource::BuiltIn
                                ? platform::assetExists(core::format("levels/level%d.json", number))
                                : SDL_GetPathInfo(userLevelPath(number).c_str(), nullptr);
        if (!exists)
            return n;
        ++n;
    }
}

std::optional<game::LevelData> GameApp::loadLevel(LevelSource source, int number) const
{
    const auto json = source == LevelSource::BuiltIn
                          ? platform::readAssetFile(core::format("levels/level%d.json", number))
                          : platform::readFile(userLevelPath(number));
    if (!json)
        return std::nullopt;
    std::string error;
    auto level = game::parseLevel(*json, &error);
    if (!level)
        SDL_Log("level %d: %s", number, error.c_str());
    return level;
}

void GameApp::showDialog(std::string text, std::vector<std::string> buttons, std::function<void(int)> onButton)
{
    m_dialog.show(uiContext(), std::move(text), std::move(buttons));
    m_dialogHandler = std::move(onButton);
}

void GameApp::showMenu()
{
    m_screen = Screen::Menu;
    m_play.reset();
    m_paused = false;

    // level 0 is the menu background ("SHEEP DOG" spelt with fences)
    if (!m_menuWorld) {
        if (auto level = loadLevel(LevelSource::BuiltIn, 0))
            m_menuWorld = std::make_unique<game::World>(*level);
    }

    std::vector<std::string> buttons{"play"};
    const bool ownLevels = levelCount(LevelSource::User) > 0;
    if (ownLevels)
        buttons.push_back("play own levels");
    showDialog("", buttons, [this, ownLevels](int button) {
        if (button == 0)
            showChooser(LevelSource::BuiltIn);
        else if (button == 1 && ownLevels)
            showChooser(LevelSource::User);
    });
}

void GameApp::showChooser(LevelSource source)
{
    const int count = levelCount(source);
    if (count == 0) {
        showDialog(source == LevelSource::BuiltIn ? "no levels found!" : "no own levels found!", {"ok"},
                   [this](int) { showMenu(); });
        return;
    }
    m_source = source;
    m_screen = Screen::Chooser;
    m_play.reset();
    m_dialog.hide();
    m_chooserCount = count;
    m_chooser.layout(count, m_renderer.screenSize());
}

void GameApp::startLevel(LevelSource source, int number)
{
    const auto level = loadLevel(source, number);
    if (!level) {
        showDialog("Oooops... error loading level!", {"ok"}, [this](int) { showChooser(m_source); });
        return;
    }
    m_source = source;
    m_levelNumber = number;
    m_screen = Screen::Play;
    m_dialog.hide();
    m_paused = false;
    m_movingDog = m_draggingDog = false;
    m_play = std::make_unique<game::LevelPlay>(*level, kOverviewSeconds);
    m_camera.zoom = 1.0f;
    updateCamera();
}

void GameApp::onLevelOver()
{
    if (m_play->status() == LevelStatus::TimeUp) {
        showDialog("time is up!", {"try again", "return"}, [this](int button) {
            if (button == 0)
                startLevel(m_source, m_levelNumber);
            else
                showChooser(m_source);
        });
        return;
    }
    const game::LevelResult r = m_play->result();
    const std::string text = core::format("level %d finished!\n\nscore: %d\ntime bonus: %d\n\ntotal score: %d",
                                          m_levelNumber, r.score, r.timeBonus, r.total());
    showDialog(text, {"play again", "next level", "stop playing"}, [this](int button) {
        if (button == 0) {
            startLevel(m_source, m_levelNumber);
        } else if (button == 1) {
            if (m_levelNumber < levelCount(m_source))
                startLevel(m_source, m_levelNumber + 1);
            else
                showDialog("all levels done!", {"ok"}, [this](int) { showChooser(m_source); });
        } else {
            showChooser(m_source);
        }
    });
}

void GameApp::pause()
{
    if (m_screen == Screen::Play && m_play && !m_play->over() && !m_dialog.visible()) {
        m_paused = true;
        stopDog();
    }
}

void GameApp::back()
{
    if (m_dialog.visible() && m_screen != Screen::Menu)
        return;
    switch (m_screen) {
    case Screen::Menu:
        m_quit = true;
        break;
    case Screen::Chooser:
        showMenu();
        break;
    case Screen::Play:
        if (m_paused)
            m_paused = false;
        else
            pause();
        break;
    }
}

// =============================================================================================
// input

void GameApp::handleEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_EVENT_WINDOW_FOCUS_LOST:
    case SDL_EVENT_WILL_ENTER_BACKGROUND:
        // (Qt: MyGraphicsScene::focusOutEvent())
        pause();
        return;
    case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_AC_BACK) {
            back();
            return;
        }
        if (event.key.key == SDLK_F3) {
            m_showCollisionRects = !m_showCollisionRects;
            return;
        }
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        if (m_screen == Screen::Play)
            m_camera.zoomAt({event.wheel.mouse_x, event.wheel.mouse_y}, std::pow(1.1f, event.wheel.y));
        return;
    default:
        break;
    }

    handlePinch(event);

    if (m_dialog.visible()) {
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
            const int button = m_dialog.hit({event.button.x, event.button.y});
            if (button >= 0) {
                m_dialog.hide();
                // the handler may open the next dialog
                auto handler = std::move(m_dialogHandler);
                m_dialogHandler = nullptr;
                if (handler)
                    handler(button);
            }
        }
        return;
    }

    if (m_screen == Screen::Chooser) {
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
            const int number = m_chooser.hit({event.button.x, event.button.y});
            if (number == 0)
                showMenu();
            else if (number > 0)
                startLevel(m_source, number);
        }
        return;
    }

    if (m_screen == Screen::Play && m_play)
        handlePlayEvent(event);
}

void GameApp::handlePinch(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_EVENT_FINGER_DOWN:
        m_fingers[static_cast<long long>(event.tfinger.fingerID)] = {event.tfinger.x, event.tfinger.y};
        break;
    case SDL_EVENT_FINGER_UP:
    case SDL_EVENT_FINGER_CANCELED:
        m_fingers.erase(static_cast<long long>(event.tfinger.fingerID));
        if (m_fingers.empty())
            m_pinching = false;
        return;
    case SDL_EVENT_FINGER_MOTION:
        m_fingers[static_cast<long long>(event.tfinger.fingerID)] = {event.tfinger.x, event.tfinger.y};
        break;
    default:
        return;
    }
    if (m_fingers.size() != 2 || m_screen != Screen::Play)
        return;

    const Vec2 a = m_fingers.begin()->second;
    const Vec2 b = std::next(m_fingers.begin())->second;
    const float dist = distance(a, b);
    if (!m_pinching) {
        // second finger down: zoom instead of steering the dog (Qt: gPinchZooming)
        m_pinching = true;
        m_pinchDistance = dist;
        stopDog();
    } else if (m_pinchDistance > 1.0f && dist > 1.0f) {
        m_camera.zoomAt((a + b) * 0.5f, dist / m_pinchDistance);
        m_pinchDistance = dist;
    }
}

void GameApp::stopDog()
{
    m_movingDog = m_draggingDog = false;
    if (m_play && !m_play->world().dogs().empty()) {
        game::Dog& dog = m_play->world().dogs()[0];
        dog.changeSpeed(0.0f);
        dog.target.reset();
    }
}

void GameApp::handlePlayEvent(const SDL_Event& event)
{
    if (m_pinching && isTouchMouse(event))
        return;
    m_hud.layout(m_renderer.screenSize());

    if (m_paused) {
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
            switch (m_hud.hit({event.button.x, event.button.y}, true)) {
            case ui::HudButton::Stop:
                showChooser(m_source);
                break;
            case ui::HudButton::Retry:
                startLevel(m_source, m_levelNumber);
                break;
            case ui::HudButton::Play:
                m_paused = false;
                break;
            default:
                break;
            }
        } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_P) {
            m_paused = false;
        }
        return;
    }
    if (m_play->status() != LevelStatus::Playing)
        return;

    game::World& w = m_play->world();
    if (w.dogs().empty())
        return;
    game::Dog& dog = w.dogs()[0];
    // drunk bonus: keyboard directions are swapped (up <-> down, left <-> right)
    const bool drunk = m_play->bonus().active(BonusMode::Drunk);

    switch (event.type) {
    case SDL_EVENT_KEY_DOWN: {
        if (event.key.repeat)
            return;
        const SDL_Keycode key = event.key.key;
        if (key == SDLK_P) {
            pause();
            return;
        }
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
        switch (m_hud.hit(m_clickPoint, false)) {
        case ui::HudButton::Pause:
            pause();
            return;
        case ui::HudButton::Bark:
            w.bark();
            return;
        default:
            break;
        }
        if (m_clickPoint.x < ui::Hud::width())
            return;
        const Vec2 levelPoint = m_camera.screenToWorld(m_clickPoint);
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
            w.walkTo(0, m_camera.screenToWorld(p));
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

// =============================================================================================
// simulation and view

void GameApp::tick()
{
    if (m_screen != Screen::Play || !m_play || m_paused || m_dialog.visible())
        return;
    m_play->tick();
    for (game::GameEvent e : m_play->world().takeEvents()) {
        // sounds come in chunk 8
        if (e == game::GameEvent::LevelWon || e == game::GameEvent::TimeUp) {
            stopDog();
            onLevelOver();
        }
    }
    if (!m_play->over())
        updateCamera();
}

void GameApp::fitCamera(const game::World& world)
{
    m_camera.viewportSize = m_renderer.screenSize();
    const Rect level{{0, 0}, world.size()};
    m_camera.zoom = std::clamp(std::min(m_camera.viewportSize.x / level.w, m_camera.viewportSize.y / level.h),
                               m_camera.minZoom, m_camera.maxZoom);
    m_camera.clampTo(level);
}

void GameApp::updateCamera()
{
    m_camera.viewportSize = m_renderer.screenSize();
    const game::World& w = m_play->world();
    const Rect levelRect{{0, 0}, w.size()};

    if (m_play->status() == LevelStatus::Overview) {
        // zoom in on the whole level (Level::overviewZooming()), then follow the dog
        const float t = m_play->overviewProgress();
        m_camera.zoom = game::kOverviewStartZoom + (1.0f - game::kOverviewStartZoom) * t;
        m_camera.position = levelRect.center() - m_camera.viewportSize / (2.0f * m_camera.zoom);
        return;
    }

    if (!w.dogs().empty()) {
        // scroll so the dog keeps a minimum distance to the screen edges (Level::checkDogInView())
        const Vec2 dog = m_camera.worldToScreen(w.dogs()[0].position());
        const float margin =
            std::min(kDogMinDistToBorder, std::min(m_camera.viewportSize.x, m_camera.viewportSize.y) / 3.0f);
        const float left = ui::Hud::width() + margin;
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
    // the info bar covers the left edge: allow scrolling the level out from under it
    m_camera.clampTo(levelRect.united({{-ui::Hud::width() / m_camera.zoom, 0}, {1, 1}}));

    // bonus boxes appear in the visible part, right of the info bar
    const Vec2 topLeft = m_camera.screenToWorld({ui::Hud::width(), 0});
    const Vec2 bottomRight = m_camera.screenToWorld(m_camera.viewportSize);
    m_play->setVisibleRect(Rect::fromCorners(topLeft, bottomRight));
}

void GameApp::drawBonus()
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

void GameApp::draw(float alpha)
{
    gfx::Renderer& r = m_renderer;
    r.clear(kBackgroundColor);
    const ui::Context ui = uiContext();

    switch (m_screen) {
    case Screen::Menu:
        if (m_menuWorld) {
            fitCamera(*m_menuWorld);
            r.setCamera(&m_camera);
            drawWorld(r, m_textures, *m_menuWorld, 1.0f);
        }
        break;
    case Screen::Chooser:
        r.setCamera(nullptr);
        m_chooser.layout(m_chooserCount, r.screenSize());
        m_chooser.draw(ui);
        break;
    case Screen::Play:
        if (m_play) {
            // characters only move while playing; don't interpolate past the last tick otherwise
            if (m_play->status() != LevelStatus::Playing || m_paused || m_dialog.visible())
                alpha = 1.0f;
            m_camera.viewportSize = r.screenSize();
            r.setCamera(&m_camera);
            drawWorld(r, m_textures, m_play->world(), alpha, m_showCollisionRects);
            drawBonus();
            r.setCamera(nullptr);
            m_hud.layout(r.screenSize());
            m_hud.draw(ui, *m_play, m_paused);
        }
        break;
    }

    r.setCamera(nullptr);
    m_dialog.draw(ui);
    r.present();
}
