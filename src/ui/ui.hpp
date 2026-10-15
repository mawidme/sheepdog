#pragma once

#include "core/color.hpp"
#include "core/rect.hpp"

#include <string>
#include <vector>

namespace gfx {
class Font;
class FontMetrics;
class Renderer;
class TextureCache;
}
namespace game { class LevelPlay; }

// Screen-space UI of the Qt version: message boxes (MyButtonBox), the info bar (InfoItems)
// with the pause screen, and the level chooser (LevelChooseItems). Layouts are computed from
// font metrics only (testable); drawing needs the renderer.
namespace ui {

// colours of the Qt version (kInfoColor etc.), text slightly more opaque for readability
inline constexpr core::Color kTextColor{0, 0, 0, 160};
inline constexpr core::Color kInfoBackground{255, 255, 255, 100};
inline constexpr float kSpacer = 7.5f;         // gGuiSpacer
inline constexpr float kInfoFontSize = 18.0f;  // gInfoFontSize
inline constexpr float kButtonFontSize = 36.0f;

struct Context {
    gfx::Renderer& renderer;
    gfx::TextureCache& textures;
    const gfx::Font& font;
};

// ---------------------------------------------------------------------------------------------
// Message box: optional text, buttons below (or stacked, if there is no text, as in the menu),
// on the cloud image (boxBackground) scaled to fit.
struct MessageBoxLayout {
    core::Rect box;                 // the background image
    core::Vec2 textTopCenter;
    std::vector<core::Rect> buttons;
    float buttonFontSize = kButtonFontSize; // smaller if the buttons wouldn't fit next to each other
};

MessageBoxLayout layoutMessageBox(const gfx::FontMetrics& metrics, const std::string& text,
                                  const std::vector<std::string>& buttons, core::Vec2 screenSize,
                                  core::Vec2 backgroundImageSize);

class MessageBox {
public:
    void show(const Context& ui, std::string text, std::vector<std::string> buttons);
    void hide() { m_visible = false; }
    bool visible() const { return m_visible; }

    // index of the button at p, or -1
    int hit(core::Vec2 p) const;
    void draw(const Context& ui);

private:
    bool m_visible = false;
    std::string m_text;
    std::vector<std::string> m_buttons;
    MessageBoxLayout m_layout;
    core::Vec2 m_screenSize;
};

// ---------------------------------------------------------------------------------------------
// Info bar on the left (vertical bars layout of the Qt version) and the pause screen.
enum class HudButton { None, Pause, Bark, Stop, Retry, Play };

class Hud {
public:
    // width of the info bar; the level view should keep the dog right of it
    static float width();

    void layout(core::Vec2 screenSize);
    HudButton hit(core::Vec2 p, bool paused) const;
    void draw(const Context& ui, const game::LevelPlay& play, bool paused) const;

private:
    core::Vec2 m_screen;
    core::Rect m_pause, m_bark, m_barkBar, m_timeBar;
    core::Vec2 m_scoreLabel, m_barkLabel, m_timeLabel;
    core::Rect m_stop, m_retry, m_play;
};

// ---------------------------------------------------------------------------------------------
// Level chooser: numbered squares, leave button at the bottom right.
class LevelChooser {
public:
    void layout(int numLevels, core::Vec2 screenSize);
    // level number (1-based), 0 for the leave button, -1 for nothing
    int hit(core::Vec2 p) const;
    void draw(const Context& ui) const;

private:
    core::Vec2 m_screen;
    std::vector<core::Rect> m_items;
    core::Rect m_leave;
};

} // namespace ui
