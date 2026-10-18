#pragma once

#include "core/vec2.hpp"
#include "game/level_data.hpp"
#include "game/level_editor.hpp"
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
namespace audio { class Audio; }

// The game: main menu, level chooser, playing a level with pause screen and dialogs.
// Replaces the Qt version's game mode handling (utils.cpp), Level and MyGraphicsScene input.
class GameApp {
public:
    // audio may be nullptr (no sound device)
    GameApp(gfx::Renderer& renderer, gfx::TextureCache& textures, audio::Audio* audio);

    void handleEvent(const SDL_Event& event); // event in render coordinates
    void tick();                              // one simulation step (1 / game::kTicksPerSecond)
    // alpha: progress towards the next tick; present = false leaves the frame unpresented (tests read it back)
    void draw(float alpha, bool present = true);
    bool wantsQuit() const { return m_quit; }

private:
    enum class Screen { Menu, Chooser, Play, Editor };
    enum class LevelSource { BuiltIn, User };
    enum class ChooserPurpose { Play, Edit };

    ui::Context uiContext() { return {m_renderer, m_textures, m_font}; }

    // --- flow ---------------------------------------------------------------------------
    void showMenu();
    void showChooser(LevelSource source, ChooserPurpose purpose = ChooserPurpose::Play);
    void startLevel(LevelSource source, int number);
    void startLevel(const game::LevelData& level);
    // where "stop" / "return" lead from a level: the chooser, or the editor when test playing
    void leaveLevel();
    void showDialog(std::string text, std::vector<std::string> buttons, std::function<void(int)> onButton);
    void back();
    void pause();
    void onLevelOver();

    int levelCount(LevelSource source) const;
    std::optional<game::LevelData> loadLevel(LevelSource source, int number) const;

    // --- editor (desktop only) -----------------------------------------------------------
    void openEditor();
    // edit a level (number of the own level it came from, 0 = none: saving makes it a new one)
    void loadIntoEditor(const game::LevelData& level, int userLevelNumber);
    void handleEditorEvent(const SDL_Event& event);
    void editorButton(ui::EditorButton button);
    // asks to save unsaved changes first, then calls next (unless the user continues editing)
    void askForSave(std::function<void()> next);
    // validates, normalizes and writes the level; false (with a dialog) if it can't be saved
    bool saveEditorLevel(bool confirm);
    void normalizeEditorLevel();
    void drawEditor();

    // --- input --------------------------------------------------------------------------
    void handlePlayEvent(const SDL_Event& event);
    void handlePinch(const SDL_Event& event);
    void stopDog();

    // --- sound ----------------------------------------------------------------------------
    // reacts to the simulation's events (sounds, end of level)
    void processGameEvents();
    void playSound(const char* name, int variants = 1);

    // --- view ---------------------------------------------------------------------------
    void updateCamera();
    void fitCamera(const game::World& world);
    void drawBonus();

    gfx::Renderer&     m_renderer;
    gfx::TextureCache& m_textures;
    audio::Audio*      m_audio;
    gfx::Font          m_font;
    gfx::Camera        m_camera;
    ui::Hud            m_hud;
    ui::LevelChooser   m_chooser;
    ui::MessageBox     m_dialog;
    std::function<void(int)> m_dialogHandler;

    Screen      m_screen = Screen::Menu;
    LevelSource m_source = LevelSource::BuiltIn;
    ChooserPurpose m_chooserPurpose = ChooserPurpose::Play;
    int         m_levelNumber = 0;
    int         m_chooserCount = 0;
    std::unique_ptr<game::World>     m_menuWorld; // level 0, the menu background
    std::unique_ptr<game::LevelPlay> m_play;
    bool m_paused = false;
    bool m_playFromEditor = false;
    game::LevelData m_testLevel; // level being test played from the editor

    game::LevelEditor  m_editor;
    ui::EditorToolbar  m_toolbar;
    bool m_editorPointerDown = false;
    bool m_panning = false;
    bool m_quit = false;
    bool m_showCollisionRects = false;

    // mouse / touch control of the dog (Qt: MyGraphicsScene)
    bool       m_movingDog = false;
    bool       m_draggingDog = false;
    core::Vec2 m_clickPoint;

    // two-finger pinch zoom (and pan in the editor)
    std::map<long long, core::Vec2> m_fingers;
    bool       m_pinching = false;
    float      m_pinchDistance = 0.0f;
    core::Vec2 m_pinchCenter;
};
