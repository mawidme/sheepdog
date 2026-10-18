#include "ui.hpp"

#include "core/strings.hpp"
#include "game/level_play.hpp"
#include "gfx/font.hpp"
#include "gfx/renderer.hpp"
#include "gfx/texture_cache.hpp"

#include <algorithm>

using core::Color;
using core::Rect;
using core::Vec2;

namespace ui {

namespace {

constexpr float kButtonFrameWidth = 3.0f;
constexpr Color kFrameColor{0, 0, 0, 100};
constexpr Color kChooserColor{60, 60, 60, 180};
constexpr Color kInactiveBackground{0, 0, 0, 150};
constexpr Color kBarkBarPen{0, 0, 150, 100};
constexpr Color kBarkBarBrush{0, 0, 180, 100};
constexpr Color kTimeBarPen{0, 80, 0, 100};
constexpr Color kBonusTimeBar{9, 150, 9, 100};
constexpr float kInfoBarWidth = 12.0f;   // gInfoBarWidth
constexpr float kInGameButtonSize = 90.0f;
constexpr float kPausedButtonSize = 120.0f;
constexpr float kChooserItemSize = 54.0f;

Vec2 buttonSize(const gfx::FontMetrics& m, const std::string& label, float fontSize)
{
    return m.measure(label, fontSize) + Vec2(2 * kSpacer, kSpacer);
}

void drawButton(const Context& ui, const Rect& r, const std::string& label, float fontSize)
{
    //ui.renderer.drawRect(r, kFrameColor, kButtonFrameWidth);
    ui.font.drawCentered(ui.renderer, label, r, fontSize, kTextColor);
}

} // namespace

// ---------------------------------------------------------------------------------------------
// message box

MessageBoxLayout layoutMessageBox(const gfx::FontMetrics& m, const std::string& text,
                                  const std::vector<std::string>& buttons, Vec2 screenSize, Vec2 imageSize)
{
    MessageBoxLayout l;
    // distance from the cloud's outline to the content (the cloud has wavy edges)
    const float frameDist = 5 * kSpacer;

    // Button labels in a row may be too wide for small screens: shrink the font until they fit.
    std::vector<Vec2> sizes;
    const float maxRowWidth = screenSize.x - 2 * frameDist - 2 * kSpacer;
    for (l.buttonFontSize = kButtonFontSize;; l.buttonFontSize -= 2.0f) {
        sizes.clear();
        float rowWidth = 2 * kSpacer * static_cast<float>(std::max<size_t>(1, buttons.size()) - 1);
        for (const std::string& b : buttons) {
            sizes.push_back(buttonSize(m, b, l.buttonFontSize));
            rowWidth += text.empty() ? 0.0f : sizes.back().x;
        }
        if (rowWidth <= maxRowWidth || l.buttonFontSize <= kInfoFontSize)
            break;
    }

    Vec2 content;
    const Vec2 textSize = text.empty() ? Vec2() : m.measure(text, kInfoFontSize);
    if (text.empty()) {
        // buttons stacked (main menu)
        for (Vec2 s : sizes) {
            content.x = std::max(content.x, s.x);
            content.y += s.y;
        }
        content.y += 2 * kSpacer * static_cast<float>(std::max<size_t>(1, sizes.size()) - 1);
    } else {
        // text on top, buttons in a row below
        float rowWidth = 0.0f, rowHeight = 0.0f;
        for (Vec2 s : sizes) {
            rowWidth += s.x;
            rowHeight = std::max(rowHeight, s.y);
        }
        rowWidth += 2 * kSpacer * static_cast<float>(std::max<size_t>(1, sizes.size()) - 1);
        content = {std::max(textSize.x, rowWidth), textSize.y + 3 * kSpacer + rowHeight};
    }

    // Scale the cloud to fit the content, keeping its aspect ratio where the screen allows;
    // otherwise squash it (wide dialogs) rather than leaving the screen.
    Vec2 boxSize = content + Vec2(2 * frameDist, 2 * frameDist);
    const float aspect = imageSize.x / imageSize.y;
    if (boxSize.x / boxSize.y < aspect)
        boxSize.x = boxSize.y * aspect;
    else
        boxSize.y = std::max(boxSize.y, std::min(boxSize.x / aspect, screenSize.y - 2 * kSpacer));
    l.box = {(screenSize - boxSize) * 0.5f, boxSize};

    const Vec2 contentTopLeft = l.box.center() - content * 0.5f;
    if (text.empty()) {
        float y = contentTopLeft.y;
        for (Vec2 s : sizes) {
            l.buttons.push_back({{l.box.center().x - s.x / 2.0f, y}, s});
            y += s.y + 2 * kSpacer;
        }
    } else {
        l.textTopCenter = {l.box.center().x, contentTopLeft.y};
        float rowWidth = 0.0f;
        for (Vec2 s : sizes)
            rowWidth += s.x;
        rowWidth += 2 * kSpacer * static_cast<float>(std::max<size_t>(1, sizes.size()) - 1);
        float x = l.box.center().x - rowWidth / 2.0f;
        const float y = contentTopLeft.y + textSize.y + 3 * kSpacer;
        for (Vec2 s : sizes) {
            l.buttons.push_back({{x, y}, s});
            x += s.x + 2 * kSpacer;
        }
    }
    return l;
}

void MessageBox::show(const Context& ui, std::string text, std::vector<std::string> buttons)
{
    m_text = std::move(text);
    m_buttons = std::move(buttons);
    m_screenSize = ui.renderer.screenSize();
    if (const gfx::FontMetrics* m = ui.font.metrics())
        m_layout = layoutMessageBox(*m, m_text, m_buttons, m_screenSize, ui.textures.get("boxBackground").size());
    m_visible = true;
}

int MessageBox::hit(Vec2 p) const
{
    if (!m_visible)
        return -1;
    for (size_t i = 0; i < m_layout.buttons.size(); ++i)
        if (m_layout.buttons[i].contains(p))
            return static_cast<int>(i);
    return -1;
}

void MessageBox::draw(const Context& ui)
{
    if (!m_visible)
        return;
    if (ui.renderer.screenSize() != m_screenSize)
        show(ui, m_text, m_buttons); // window resized: lay out again
    ui.renderer.drawTexture(ui.textures.get("boxBackground"), m_layout.box);
    if (!m_text.empty())
        ui.font.draw(ui.renderer, m_text, m_layout.textTopCenter, kInfoFontSize, kTextColor, gfx::TextAlign::Center);
    for (size_t i = 0; i < m_buttons.size() && i < m_layout.buttons.size(); ++i)
        drawButton(ui, m_layout.buttons[i], m_buttons[i], m_layout.buttonFontSize);
}

// ---------------------------------------------------------------------------------------------
// info bar and pause screen

float Hud::width()
{
    return kInGameButtonSize + 2 * kSpacer;
}

void Hud::layout(Vec2 screen)
{
    m_screen = screen;
    // pause button top, bark button bottom, score and the two bars in between
    m_pause = {kSpacer, kSpacer, kInGameButtonSize, kInGameButtonSize};
    m_bark = {kSpacer, screen.y - kInGameButtonSize - kSpacer, kInGameButtonSize, kInGameButtonSize};
    m_scoreLabel = {width() / 2.0f, m_pause.bottom() + kSpacer};
    const float barsTop = m_scoreLabel.y + 2 * 25.0f + 2 * kSpacer;
    m_barkLabel = {width() / 2.0f - 15.0f, barsTop};
    m_timeLabel = {width() / 2.0f + 15.0f, barsTop};
    const float barTop = barsTop + 25.0f;
    const float barLength = std::max(40.0f, m_bark.top() - kSpacer - barTop);
    m_barkBar = {m_barkLabel.x - kInfoBarWidth / 2.0f, barTop, kInfoBarWidth, barLength};
    m_timeBar = {m_timeLabel.x - kInfoBarWidth / 2.0f, barTop, kInfoBarWidth, barLength};

    // pause screen: stop, retry, play
    const float y = (screen.y - kPausedButtonSize) / 2.0f;
    const float cx = (Hud::width() + screen.x) / 2.0f;
    m_stop = {cx - 2.0f * kPausedButtonSize, y, kPausedButtonSize, kPausedButtonSize};
    m_retry = {cx - 0.5f * kPausedButtonSize, y, kPausedButtonSize, kPausedButtonSize};
    m_play = {cx + kPausedButtonSize, y, kPausedButtonSize, kPausedButtonSize};
}

HudButton Hud::hit(Vec2 p, bool paused) const
{
    if (paused) {
        if (m_stop.contains(p))
            return HudButton::Stop;
        if (m_retry.contains(p))
            return HudButton::Retry;
        if (m_play.contains(p))
            return HudButton::Play;
        return HudButton::None;
    }
    if (m_pause.contains(p))
        return HudButton::Pause;
    if (m_bark.contains(p))
        return HudButton::Bark;
    return HudButton::None;
}

void Hud::draw(const Context& ui, const game::LevelPlay& play, bool paused) const
{
    gfx::Renderer& r = ui.renderer;
    r.fillRect({0, 0, width(), m_screen.y}, kInfoBackground);
    r.drawTexture(ui.textures.get("pauseButton"), m_pause);
    r.drawTexture(ui.textures.get("barkButton"), m_bark);

    ui.font.draw(r, "SCORE:", m_scoreLabel, kInfoFontSize, kTextColor, gfx::TextAlign::Center);
    ui.font.draw(r, core::format("%d", play.score()), m_scoreLabel + Vec2(0, 25.0f), kInfoFontSize, kTextColor,
                 gfx::TextAlign::Center);
    ui.font.draw(r, "B", m_barkLabel, kInfoFontSize, kTextColor, gfx::TextAlign::Center);
    ui.font.draw(r, "T", m_timeLabel, kInfoFontSize, kTextColor, gfx::TextAlign::Center);

    // bars fill from the bottom
    const auto& dogs = play.world().dogs();
    const float power = dogs.empty() ? 0.0f : dogs[0].barkPower() / game::kMaxBarkPower;
    const float barkH = m_barkBar.h * power;
    r.fillRect({m_barkBar.x, m_barkBar.bottom() - barkH, m_barkBar.w, barkH}, kBarkBarBrush);
    r.drawRect(m_barkBar, kBarkBarPen, 1.0f);

    // time: red when running out, green when full (the Qt version used a gradient); bonus on top
    const game::LevelTime& t = play.time();
    const float total = static_cast<float>(std::max(1, t.total));
    const float left = std::clamp(static_cast<float>(t.left) / total, 0.0f, 1.0f);
    const float bonus = std::clamp(static_cast<float>(t.bonus) / total, 0.0f, 1.0f - left);
    const auto lerp = [](float a, float b, float f) { return static_cast<std::uint8_t>(a + (b - a) * f); };
    const Color timeColor{lerp(150, 0, left), lerp(0, 150, left), 0, 100};
    const float leftH = m_timeBar.h * left;
    const float bonusH = m_timeBar.h * bonus;
    r.fillRect({m_timeBar.x, m_timeBar.bottom() - leftH, m_timeBar.w, leftH}, timeColor);
    r.fillRect({m_timeBar.x, m_timeBar.bottom() - leftH - bonusH, m_timeBar.w, bonusH}, kBonusTimeBar);
    r.drawRect(m_timeBar, kTimeBarPen, 1.0f);

    if (paused) {
        r.fillRect({{0, 0}, m_screen}, kInactiveBackground);
        r.drawTexture(ui.textures.get("stopButton"), m_stop);
        r.drawTexture(ui.textures.get("retryButton"), m_retry);
        r.drawTexture(ui.textures.get("playButton"), m_play);
    }
}

// ---------------------------------------------------------------------------------------------
// level chooser

void LevelChooser::layout(int numLevels, Vec2 screen)
{
    m_screen = screen;
    m_items.clear();
    const Rect frame{3, 3, screen.x - 6, screen.y - 6};
    float x = frame.x + kSpacer, y = frame.y + kSpacer;
    for (int i = 0; i < numLevels; ++i) {
        if (x + kChooserItemSize > frame.right() - kSpacer) {
            x = frame.x + kSpacer;
            y += kChooserItemSize + kSpacer;
        }
        m_items.push_back({x, y, kChooserItemSize, kChooserItemSize});
        x += kChooserItemSize + kSpacer;
    }
    const Vec2 leaveSize{25.650753f, 50.0f}; // img/editor/leave.svg
    m_leave = {frame.bottomRight() - leaveSize - Vec2(kSpacer, kSpacer), leaveSize};
}

int LevelChooser::hit(Vec2 p) const
{
    for (size_t i = 0; i < m_items.size(); ++i)
        if (m_items[i].contains(p))
            return static_cast<int>(i) + 1;
    return m_leave.inflated(kSpacer).contains(p) ? 0 : -1;
}

void LevelChooser::draw(const Context& ui) const
{
    gfx::Renderer& r = ui.renderer;
    r.drawRect({3, 3, m_screen.x - 6, m_screen.y - 6}, kChooserColor, kButtonFrameWidth);
    for (size_t i = 0; i < m_items.size(); ++i) {
        r.drawRect(m_items[i], kChooserColor, kButtonFrameWidth);
        ui.font.drawCentered(r, core::format("%d", static_cast<int>(i) + 1), m_items[i], kButtonFontSize, kChooserColor);
    }
    r.drawTexture(ui.textures.get("editor/leave"), m_leave);
}

// ---------------------------------------------------------------------------------------------
// editor toolbar

namespace {

constexpr float kToolSize = 54.0f;
constexpr const char* kToolImages[static_cast<int>(EditorButton::Count)] = {
    "editor/fence", "editor/bush",   "editor/tree", "editor/water", "editor/exit", "editor/dog",  "editor/sheep",
    "editor/undo",  "editor/eraser", "editor/pan",  "editor/load",  "editor/save", "editor/play", "editor/leave"};

} // namespace

float EditorToolbar::columnWidth()
{
    return kToolSize + 2 * kSpacer;
}

void EditorToolbar::layout(Vec2 screen)
{
    const int firstRight = static_cast<int>(EditorButton::Undo);
    for (int i = 0; i < static_cast<int>(EditorButton::Count); ++i) {
        const bool right = i >= firstRight;
        const int row = right ? i - firstRight : i;
        const float x = right ? screen.x - kToolSize - kSpacer : kSpacer;
        m_buttons[i] = {x, kSpacer + static_cast<float>(row) * (kToolSize + kSpacer), kToolSize, kToolSize};
    }
}

EditorButton EditorToolbar::hit(Vec2 p) const
{
    for (int i = 0; i < static_cast<int>(EditorButton::Count); ++i)
        if (m_buttons[i].contains(p))
            return static_cast<EditorButton>(i);
    return EditorButton::Count;
}

void EditorToolbar::draw(const Context& ui, EditorButton selectedTool) const
{
    gfx::Renderer& r = ui.renderer;
    for (int i = 0; i < static_cast<int>(EditorButton::Count); ++i) {
        const Rect& b = m_buttons[i];
        r.fillRect(b, Color(100, 100, 100, 180));
        r.drawRect(b, Color(60, 60, 60, 180), 2.0f);
        if (static_cast<EditorButton>(i) == selectedTool) {
            r.fillRect(b, Color(227, 224, 61, 100));
            r.drawRect(b, Color(165, 163, 47, 150), 2.0f);
        }
        const gfx::Texture& icon = ui.textures.get(kToolImages[i]);
        r.drawTexture(icon, {b.center() - icon.size() * 0.5f, icon.size()});
    }
}

} // namespace ui
