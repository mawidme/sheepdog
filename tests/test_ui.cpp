// Tests for the SDL-free parts of the UI: font metrics/layout, message box, HUD and chooser layout.
#include "gfx/font.hpp"
#include "ui/ui.hpp"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

using core::Rect;
using core::Vec2;

namespace {

int gFailures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::printf("%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
            ++gFailures;                                                         \
        }                                                                        \
    } while (0)

bool near(float a, float b, float eps = 0.5f) { return std::fabs(a - b) <= eps; }

std::string readFile(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::optional<gfx::FontMetrics> loadMetrics(int scale)
{
    return gfx::FontMetrics::parse(
        readFile(std::string(SHEEPDOG_ASSETS_DIR) + "/fonts/" + std::to_string(scale) + "x/ui.json"),
        static_cast<float>(scale));
}

bool inside(const Rect& outer, const Rect& inner) { return outer.contains(inner); }

void testFontMetrics()
{
    CHECK(!gfx::FontMetrics::parse("{}", 1.0f).has_value());
    CHECK(!gfx::FontMetrics::parse("not json", 1.0f).has_value());

    const auto m1 = loadMetrics(1);
    const auto m4 = loadMetrics(4);
    CHECK(m1.has_value() && m4.has_value());
    if (!m1 || !m4)
        return;

    // the same text has (nearly) the same logical size at every atlas scale
    const Vec2 a = m1->measure("level 3 finished!", 18.0f);
    const Vec2 b = m4->measure("level 3 finished!", 18.0f);
    CHECK(near(a.x, b.x, 2.0f) && near(a.y, b.y, 1.0f));
    CHECK(a.x > 100.0f && a.x < 250.0f);

    // size scales linearly, lines stack
    const Vec2 big = m1->measure("level 3 finished!", 36.0f);
    CHECK(near(big.x, 2 * a.x, 1.0f));
    const Vec2 two = m1->measure("score: 10\ntime bonus: 99", 18.0f);
    CHECK(near(two.y, 2 * m1->lineHeight(18.0f), 0.01f));
    CHECK(near(two.x, m1->measure("time bonus: 99", 18.0f).x, 0.01f));
    CHECK(m1->measure("", 18.0f).x == 0.0f);

    // layout: one quad per visible glyph, alignment per line
    std::vector<gfx::GlyphQuad> quads;
    m1->layout("a b", {100, 50}, 18.0f, gfx::TextAlign::Left, quads);
    CHECK(quads.size() == 2);
    CHECK(quads[0].dest.x >= 99.0f && quads[0].dest.y >= 50.0f);
    CHECK(quads[1].dest.x > quads[0].dest.right());

    quads.clear();
    m1->layout("wide line\nx", {200, 0}, 18.0f, gfx::TextAlign::Center, quads);
    const Rect& x = quads.back().dest;
    CHECK(near(x.center().x, 200.0f, 2.0f));
    CHECK(x.y > m1->lineHeight(18.0f) * 0.9f);

    quads.clear();
    m1->layout("end", {300, 0}, 18.0f, gfx::TextAlign::Right, quads);
    CHECK(quads.back().dest.right() <= 300.5f && quads.back().dest.right() > 290.0f);

    // unknown characters fall back to '?'
    quads.clear();
    m1->layout("\x01", {0, 0}, 18.0f, gfx::TextAlign::Left, quads);
    CHECK(quads.size() == 1);
}

void testMessageBox()
{
    const auto m = loadMetrics(1);
    CHECK(m.has_value());
    if (!m)
        return;
    const Vec2 screen{800, 480};
    const Vec2 cloud{300, 200};

    // with text: buttons in one row below the text, all inside the box, box centred
    const auto l = ui::layoutMessageBox(*m, "level 1 finished!\n\nscore: 10\ntime bonus: 99\n\ntotal score: 109",
                                        {"play again", "next level", "stop playing"}, screen, cloud);
    CHECK(l.buttons.size() == 3);
    CHECK(near(l.box.center().x, 400.0f) && near(l.box.center().y, 240.0f));
    CHECK(Rect({0, 0}, screen).contains(l.box)); // smaller button font / squashed cloud rather than off screen
    CHECK(l.buttonFontSize < ui::kButtonFontSize);
    const auto wide = ui::layoutMessageBox(*m, "time is up!", {"try again", "return"}, {1077, 480}, cloud);
    CHECK(wide.buttonFontSize == ui::kButtonFontSize && Rect({0, 0}, {1077, 480}).contains(wide.box));
    for (const Rect& b : l.buttons) {
        CHECK(inside(l.box, b));
        CHECK(near(b.y, l.buttons[0].y));
        CHECK(b.y > l.textTopCenter.y + 5 * m->lineHeight(ui::kInfoFontSize));
    }
    CHECK(l.buttons[0].right() < l.buttons[1].left() && l.buttons[1].right() < l.buttons[2].left());

    // no text (main menu): buttons stacked and horizontally centred
    const auto menu = ui::layoutMessageBox(*m, "", {"play", "play own levels"}, screen, cloud);
    CHECK(menu.buttons.size() == 2);
    CHECK(menu.buttons[0].bottom() < menu.buttons[1].top());
    CHECK(near(menu.buttons[0].center().x, 400.0f) && near(menu.buttons[1].center().x, 400.0f));
    for (const Rect& b : menu.buttons)
        CHECK(inside(menu.box, b));
    CHECK(near(menu.box.w / menu.box.h, 1.5f, 0.01f)); // keeps the cloud's shape when there is room
}

void testHudAndChooser()
{
    ui::Hud hud;
    hud.layout({1077, 480});
    CHECK(hud.hit({50, 50}, false) == ui::HudButton::Pause);
    CHECK(hud.hit({50, 440}, false) == ui::HudButton::Bark);
    CHECK(hud.hit({600, 240}, false) == ui::HudButton::None);
    CHECK(hud.hit({50, 50}, true) == ui::HudButton::None); // paused: only the pause screen buttons
    // the three pause screen buttons are in a row in the middle of the level area
    int found = 0;
    for (float x = ui::Hud::width(); x < 1077; x += 5) {
        const ui::HudButton b = hud.hit({x, 240}, true);
        found |= b == ui::HudButton::Stop ? 1 : b == ui::HudButton::Retry ? 2 : b == ui::HudButton::Play ? 4 : 0;
    }
    CHECK(found == 7);

    ui::LevelChooser chooser;
    chooser.layout(30, {800, 480});
    CHECK(chooser.hit({10 + 27, 10 + 27}) == 1);
    CHECK(chooser.hit({800 - 20, 480 - 30}) == 0); // leave button
    CHECK(chooser.hit({400, 470}) == -1);
    // second row starts with the level after the last one fitting in the first
    int maxFirstRow = 0;
    for (float x = 10; x < 790; x += 10)
        maxFirstRow = std::max(maxFirstRow, chooser.hit({x, 37}));
    CHECK(maxFirstRow > 5 && chooser.hit({37, 37 + 54 + 7.5f}) == maxFirstRow + 1);
}

} // namespace

int main()
{
    testFontMetrics();
    testMessageBox();
    testHudAndChooser();

    if (gFailures == 0)
        std::printf("all ui tests passed\n");
    return gFailures == 0 ? 0 : 1;
}
