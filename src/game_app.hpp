#pragma once

#include "core/vec2.hpp"
#include "game/level_data.hpp"
#include "game/level_play.hpp"
#include "gfx/camera.hpp"
#include "gfx/font.hpp"
#include "ui/ui.hpp"

#include <functional>
#include <map>
#include <memory>
#include <optional>

union SDL_Event;

namespace gfx {
class Renderer;
class TextureCache;
}

// The game: main menu, level chooser, playing a level with pause screen and dialogs.
// Replaces the Qt version's game mode handling (utils.cpp), Level and MyGraphicsScene input.
class GameApp {
public:
    GameApp(gfx::Renderer& renderer, gfx::TextureCache& textures);

    void handleEvent(const SDL_Event& event); // event in render coordinates
    void tick();                              // one simulation step (1 / game::kTicksPerSecond)
    void draw(float alpha);                   // alpha: progress towards the next tick
    bool wantsQuit() const { return m_quit; }

private:
    enum class Screen { Menu, Chooser, Play };
    enum class LevelSource { BuiltIn, User };

    ui::Context uiContext() { return {m_renderer, m_textures, m_font}; }

    // --- flow ---------------------------------------------------------------------------
    void showMenu();
    void showChooser(LevelSource source);
    void startLevel(LevelSource source, int number);
    void showDialog(std::string text, std::vector<std::string> buttons, std::function<void(int)> onButton);
    void back();
    void pause();
    void onLevelOver();

    int levelCount(LevelSource source) const;
    std::optional<game::LevelData> loadLevel(LevelSource source, int number) const;

    // --- input --------------------------------------------------------------------------
    void handlePlayEvent(const SDL_Event& event);
    void handlePinch(const SDL_Event& event);
    void stopDog();

    // --- view ---------------------------------------------------------------------------
    void updateCamera();
    void fitCamera(const game::World& world);
    void drawBonus();

    gfx::Renderer&     m_renderer;
    gfx::TextureCache& m_textures;
    gfx::Font          m_font;
    gfx::Camera        m_camera;
    ui::Hud            m_hud;
    ui::LevelChooser   m_chooser;
    ui::MessageBox     m_dialog;
    std::function<void(int)> m_dialogHandler;

    Screen      m_screen = Screen::Menu;
    LevelSource m_source = LevelSource::BuiltIn;
    int         m_levelNumber = 0;
    int         m_chooserCount = 0;
    std::unique_ptr<game::World>     m_menuWorld; // level 0, the menu background
    std::unique_ptr<game::LevelPlay> m_play;
    bool m_paused = false;
    bool m_quit = false;
    bool m_showCollisionRects = false;

    // mouse / touch control of the dog (Qt: MyGraphicsScene)
    bool       m_movingDog = false;
    bool       m_draggingDog = false;
    core::Vec2 m_clickPoint;

    // two-finger pinch zoom
    std::map<long long, core::Vec2> m_fingers;
    bool  m_pinching = false;
    float m_pinchDistance = 0.0f;
};
