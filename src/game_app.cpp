#include "game_app.hpp"

#include "audio/audio.hpp"
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

game::EditTool toEditTool(ui::EditorButton b)
{
    switch (b) {
    case ui::EditorButton::Eraser:
        return game::EditTool::Eraser;
    case ui::EditorButton::Pan:
        return game::EditTool::Pan;
    default:
        // Fence .. Sheep are in the same order
        return static_cast<game::EditTool>(static_cast<int>(b));
    }
}

ui::EditorButton toEditorButton(game::EditTool t)
{
    switch (t) {
    case game::EditTool::Eraser:
        return ui::EditorButton::Eraser;
    case game::EditTool::Pan:
        return ui::EditorButton::Pan;
    default:
        return static_cast<ui::EditorButton>(static_cast<int>(t));
    }
}

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

// number of level start jingles (sfx/level1..4); levels beyond that reuse them
constexpr int kNumLevelJingles = 4;

GameApp::GameApp(gfx::Renderer& renderer, gfx::TextureCache& textures, audio::Audio* audio)
    : m_renderer(renderer),
      m_textures(textures),
      m_audio(audio),
      m_font(gfx::Font::load(renderer.sdl(), textures.scale()))
{
    if (m_audio) {
        for (const auto& [name, variants] : {std::pair<const char*, int>{"bark", 4}, {"sheep", 4}, {"rescue", 2},
                                             {"level", kNumLevelJingles}, {"noBark", 1}, {"barkPowerCritical", 1},
                                             {"barkPowerFull", 1}, {"tryAgain", 1}, {"levelDone", 1}})
            m_audio->preload(name, variants);
    }
    showMenu();
}

void GameApp::playSound(const char* name, int variants)
{
    if (m_audio)
        m_audio->play(name, variants);
}

void GameApp::processGameEvents()
{
    if (!m_play)
        return;
    for (game::GameEvent e : m_play->world().takeEvents()) {
        switch (e) {
        case game::GameEvent::Bark:
            playSound("bark", 4);
            break;
        case game::GameEvent::NoBark:
            playSound("noBark");
            break;
        case game::GameEvent::BarkPowerCritical:
            playSound("barkPowerCritical");
            break;
        case game::GameEvent::BarkPowerFull:
            playSound("barkPowerFull");
            break;
        case game::GameEvent::SheepBaa:
            playSound("sheep", 4);
            break;
        case game::GameEvent::SheepRescued:
            playSound("rescue", 2);
            break;
        case game::GameEvent::TimeUp:
            playSound("tryAgain");
            stopDog();
            onLevelOver();
            return; // the dialog may start a new level
        case game::GameEvent::LevelWon:
            // (the Qt version loaded levelDone.wav but never played it)
            playSound("levelDone");
            stopDog();
            onLevelOver();
            return;
        case game::GameEvent::BonusPickedUp:
        case game::GameEvent::BonusEnded:
            break;
        }
    }
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
    if (levelCount(LevelSource::User) > 0)
        buttons.push_back("play own levels");
        buttons.push_back("editor");
    showDialog("", buttons, [this, buttons](int button) {
        const std::string& b = buttons[static_cast<size_t>(button)];
        if (b == "play")
            showChooser(LevelSource::BuiltIn);
        else if (b == "play own levels")
            showChooser(LevelSource::User);
        else if (b == "editor")
            openEditor();
    });
}

void GameApp::showChooser(LevelSource source, ChooserPurpose purpose)
{
    const int count = levelCount(source);
    if (count == 0) {
        showDialog(source == LevelSource::BuiltIn ? "no levels found!" : "no own levels found!", {"ok"},
                   [this, purpose](int) {
                       if (purpose == ChooserPurpose::Edit)
                           m_screen = Screen::Editor;
                       else
                           showMenu();
                   });
        return;
    }
    m_source = source;
    m_chooserPurpose = purpose;
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
    m_playFromEditor = false;
    m_screen = Screen::Play;
    m_dialog.hide();
    m_paused = false;
    m_movingDog = m_draggingDog = false;
    m_play = std::make_unique<game::LevelPlay>(*level, kOverviewSeconds);
    m_camera.zoom = 1.0f;
    updateCamera();
    // jingle at the start of a level (Level::start(); not when test playing from the editor)
    if (m_audio)
        m_audio->play(core::format("level%d", (number - 1) % kNumLevelJingles + 1));
}

void GameApp::startLevel(const game::LevelData& level)
{
    m_testLevel = level;
    m_playFromEditor = true;
    m_screen = Screen::Play;
    m_dialog.hide();
    m_paused = false;
    m_movingDog = m_draggingDog = false;
    m_play = std::make_unique<game::LevelPlay>(level, kOverviewSeconds);
    m_camera.zoom = 1.0f;
    updateCamera();
}

void GameApp::leaveLevel()
{
    m_play.reset();
    m_paused = false;
    if (m_playFromEditor) {
        m_playFromEditor = false;
        m_screen = Screen::Editor;
        m_dialog.hide();
        m_camera.zoom = 1.0f;
        m_camera.position = {-ui::EditorToolbar::columnWidth(), 0};
    } else {
        showChooser(m_source);
    }
}

void GameApp::onLevelOver()
{
    const auto restart = [this] {
        if (m_playFromEditor)
            startLevel(m_testLevel);
        else
            startLevel(m_source, m_levelNumber);
    };
    if (m_play->status() == LevelStatus::TimeUp) {
        showDialog("time is up!", {"try again", "return"}, [this, restart](int button) {
            if (button == 0)
                restart();
            else
                leaveLevel();
        });
        return;
    }
    const game::LevelResult r = m_play->result();
    if (m_playFromEditor) {
        // (Level::finish() in edit mode)
        const std::string text = core::format("level finished!\n\nscore: %d\ntime bonus: %d\n\ntotal score: %d",
                                              r.score, r.timeBonus, r.total());
        showDialog(text, {"play again", "return to editor"}, [this, restart](int button) {
            if (button == 0)
                restart();
            else
                leaveLevel();
        });
        return;
    }
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
        if (m_chooserPurpose == ChooserPurpose::Edit)
            m_screen = Screen::Editor;
        else
            showMenu();
        break;
    case Screen::Editor:
        editorButton(ui::EditorButton::Leave);
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
        // (Qt: MyGraphicsScene::focusOutEvent())
        pause();
        return;
    case SDL_EVENT_WILL_ENTER_BACKGROUND:
        pause();
        if (m_audio)
            m_audio->pauseDevice();
        return;
    case SDL_EVENT_DID_ENTER_FOREGROUND:
        if (m_audio)
            m_audio->resumeDevice();
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
        if (event.key.key == SDLK_M && m_audio) {
            m_audio->setMuted(!m_audio->muted());
            return;
        }
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        if (m_screen == Screen::Play || m_screen == Screen::Editor)
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
            if (m_chooserPurpose == ChooserPurpose::Edit) {
                // load an own level into the editor
                if (number > 0) {
                    if (auto level = loadLevel(LevelSource::User, number))
                        loadIntoEditor(*level, number);
                }
                if (number >= 0)
                    m_screen = Screen::Editor;
            } else if (number == 0) {
                showMenu();
            } else if (number > 0) {
                startLevel(m_source, number);
            }
        }
        return;
    }

    if (m_screen == Screen::Play && m_play) {
        handlePlayEvent(event);
        // barking etc. sounds right away, not only with the next tick
        processGameEvents();
    }
    else if (m_screen == Screen::Editor)
        handleEditorEvent(event);
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
    if (m_fingers.size() != 2 || (m_screen != Screen::Play && m_screen != Screen::Editor))
        return;

    const Vec2 a = m_fingers.begin()->second;
    const Vec2 b = std::next(m_fingers.begin())->second;
    const Vec2 center = (a + b) * 0.5f;
    const float dist = distance(a, b);
    if (!m_pinching) {
        // second finger down: zoom instead of steering the dog (Qt: gPinchZooming) or editing
        m_pinching = true;
        m_pinchDistance = dist;
        m_pinchCenter = center;
        if (m_screen == Screen::Play) {
            stopDog();
        } else {
            // the first finger already pressed with the current tool: take that back
            if (m_editorPointerDown && !m_panning)
                m_editor.cancel();
            m_editorPointerDown = m_panning = false;
        }
    } else {
        // in the editor two fingers also pan (the camera follows the dog while playing)
        if (m_screen == Screen::Editor)
            m_camera.panByScreen(center - m_pinchCenter);
        if (m_pinchDistance > 1.0f && dist > 1.0f)
            m_camera.zoomAt(center, dist / m_pinchDistance);
        m_pinchDistance = dist;
        m_pinchCenter = center;
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
                leaveLevel();
                break;
            case ui::HudButton::Retry:
                if (m_playFromEditor)
                    startLevel(m_testLevel);
                else
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
    processGameEvents();
    if (m_play && !m_play->over())
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

void GameApp::draw(float alpha, bool present)
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
    case Screen::Editor:
        drawEditor();
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
            if (m_play->bonus().active(BonusMode::Drunk)) {
                // drunk bonus: a half transparent copy of the view wobbles on top
                // (Qt: MyGraphicsScene::handleDrunkMode(), offset in screen pixels per frame)
                const float f = static_cast<float>(m_play->world().tickCount()) + alpha;
                const Vec2 wobble{12.0f * std::cos(f / 10.0f), 5.0f * std::cos(f / 14.0f)};
                gfx::Camera ghost = m_camera;
                ghost.position -= wobble / ghost.zoom;
                r.setCamera(&ghost);
                r.setOpacity(0.5f);
                drawWorld(r, m_textures, m_play->world(), alpha, false, false);
                drawBonus();
                r.setOpacity(1.0f);
                r.setCamera(&m_camera);
            }
            r.setCamera(nullptr);
            m_hud.layout(r.screenSize());
            m_hud.draw(ui, *m_play, m_paused);
        }
        break;
    }

    r.setCamera(nullptr);
    m_dialog.draw(ui);
    if (present)
        r.present();
}

// =============================================================================================
// level editor (Qt: the editing part of Level, EditorItems)

void GameApp::openEditor()
{
    m_screen = Screen::Editor;
    m_dialog.hide();
    m_editor.clear();
    m_editor.tool = game::EditTool::Fence;
    m_camera.zoom = 1.0f;
    // level origin right of the tool column
    m_camera.position = {-ui::EditorToolbar::columnWidth(), 0};
}

void GameApp::loadIntoEditor(const game::LevelData& level, int userLevelNumber)
{
    m_editor.load(level, userLevelNumber);
    m_camera.zoom = 1.0f;
    m_camera.position = {-ui::EditorToolbar::columnWidth(), 0};
}

void GameApp::normalizeEditorLevel()
{
    // objects move so the level starts at (0, 0); move the view along so nothing jumps
    m_camera.position += m_editor.normalize();
}

bool GameApp::saveEditorLevel(bool confirm)
{
    if (const auto error = m_editor.validate()) {
        showDialog(*error, {"ok"}, nullptr);
        return false;
    }
    normalizeEditorLevel();

    int number = m_editor.userLevelNumber();
    if (number == 0)
        number = levelCount(LevelSource::User) + 1;
    const std::string json = game::serializeLevel(m_editor.level());
    if (!platform::writeFile(userLevelPath(number), json)) {
        showDialog("error saving the level!", {"ok"}, nullptr);
        return false;
    }
    m_editor.setUserLevelNumber(number);
    m_editor.markSaved();
#ifdef SDL_PLATFORM_EMSCRIPTEN
    // the file above lasts only until the page is closed: the player keeps the level as a download
    const std::string fileName = core::format("level%d.json", number);
    platform::downloadFile(fileName, json);
    if (confirm)
        showDialog(core::format("saved as own level %d\nand downloaded as %s", number, fileName.c_str()), {"ok"},
                   nullptr);
#else
    if (confirm)
        showDialog(core::format("saved as own level %d", number), {"ok"}, nullptr);
#endif
    return true;
}

void GameApp::askForSave(std::function<void()> next)
{
    if (!m_editor.dirty()) {
        next();
        return;
    }
    // (Level::askForSave())
    showDialog("level has not been saved!\n\nwhat do you want to do?", {"save now", "don't save", "continue editing"},
               [this, next](int button) {
                   if (button == 0) {
                       if (saveEditorLevel(false))
                           next();
                   } else if (button == 1) {
                       next();
                   }
               });
}

void GameApp::editorButton(ui::EditorButton button)
{
    switch (button) {
    case ui::EditorButton::Undo:
        m_editor.undo();
        break;
    case ui::EditorButton::Load:
#ifdef SDL_PLATFORM_EMSCRIPTEN
        // own levels are files on the player's device (see saveEditorLevel()): pick one in the browser
        askForSave([this] {
            platform::pickFile(".json,application/json", [this](std::string json) {
                if (m_screen != Screen::Editor)
                    return; // left the editor while the dialog was open
                std::string error;
                if (auto level = game::parseLevel(json, &error))
                    loadIntoEditor(*level, 0);
                else
                    showDialog("this is not a level file:\n" + error, {"ok"}, nullptr);
            });
        });
#else
        askForSave([this] { showChooser(LevelSource::User, ChooserPurpose::Edit); });
#endif
        break;
    case ui::EditorButton::Save:
        saveEditorLevel(true);
        break;
    case ui::EditorButton::Play:
        if (const auto error = m_editor.validate()) {
            showDialog(*error, {"ok"}, nullptr);
        } else {
            normalizeEditorLevel();
            startLevel(m_editor.level());
        }
        break;
    case ui::EditorButton::Leave:
        askForSave([this] { showMenu(); });
        break;
    case ui::EditorButton::Count:
        break;
    default:
        m_editor.tool = toEditTool(button);
        break;
    }
}

void GameApp::handleEditorEvent(const SDL_Event& event)
{
    if (m_pinching && isTouchMouse(event))
        return;
    m_toolbar.layout(m_renderer.screenSize());
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_Z && (event.key.mod & SDL_KMOD_CTRL))
            m_editor.undo();
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        const Vec2 p{event.button.x, event.button.y};
        if (event.button.button != SDL_BUTTON_LEFT) {
            // right or middle button: pan with any tool
            m_panning = true;
            break;
        }
        const ui::EditorButton b = m_toolbar.hit(p);
        if (b != ui::EditorButton::Count) {
            editorButton(b);
            break;
        }
        m_editorPointerDown = true;
        if (m_editor.tool == game::EditTool::Pan)
            m_panning = true;
        else
            m_editor.press(m_camera.screenToWorld(p));
        break;
    }
    case SDL_EVENT_MOUSE_MOTION:
        if (m_panning)
            m_camera.panByScreen({event.motion.xrel, event.motion.yrel});
        else if (m_editorPointerDown)
            m_editor.drag(m_camera.screenToWorld({event.motion.x, event.motion.y}));
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (m_editorPointerDown && !m_panning)
            m_editor.release();
        m_editorPointerDown = false;
        m_panning = false;
        break;
    default:
        break;
    }
}

void GameApp::drawEditor()
{
    gfx::Renderer& r = m_renderer;
    m_camera.viewportSize = r.screenSize();
    r.setCamera(&m_camera);
    // the level's extent as it would be saved
    const Rect bounds = m_editor.contentBounds();
    if (!bounds.isNull())
        r.drawRect(bounds.inflated(2.0f), Color(255, 255, 255, 60), 2.0f);
    drawLevelData(r, m_textures, m_editor.level());

    r.setCamera(nullptr);
    m_toolbar.layout(r.screenSize());
    m_toolbar.draw(uiContext(), toEditorButton(m_editor.tool));
}
