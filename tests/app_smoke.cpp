// Smoke test of the whole app without a screen: runs GameApp with SDL's offscreen video driver,
// feeds it scripted mouse input (menu -> editor -> build a level -> save -> test play -> back)
// and checks the outcome. Screenshots of each step are written to the working directory
// (smoke_*.png) for a visual check.
#include "audio/audio.hpp"
#include "game/level_data.hpp"
#include "game_app.hpp"
#include "gfx/font.hpp"
#include "gfx/renderer.hpp"
#include "gfx/texture_cache.hpp"
#include "platform/assets.hpp"
#include "ui/ui.hpp"

#include <SDL3/SDL.h>

#include <cstdio>
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

constexpr Vec2 kScreen{1077, 480};

struct Harness {
    SDL_Renderer* sdl;
    gfx::Renderer& renderer;
    gfx::TextureCache& textures;
    gfx::Font& font;
    GameApp& app;

    void send(SDL_Event e) { app.handleEvent(e); }

    void mouse(SDL_EventType type, Vec2 p, Vec2 rel = {})
    {
        SDL_Event e{};
        e.type = type;
        if (type == SDL_EVENT_MOUSE_MOTION) {
            e.motion.x = p.x;
            e.motion.y = p.y;
            e.motion.xrel = rel.x;
            e.motion.yrel = rel.y;
        } else {
            e.button.button = SDL_BUTTON_LEFT;
            e.button.x = p.x;
            e.button.y = p.y;
        }
        send(e);
    }

    void click(Vec2 p)
    {
        mouse(SDL_EVENT_MOUSE_BUTTON_DOWN, p);
        mouse(SDL_EVENT_MOUSE_BUTTON_UP, p);
    }

    void dragLine(Vec2 from, Vec2 to)
    {
        mouse(SDL_EVENT_MOUSE_BUTTON_DOWN, from);
        Vec2 last = from;
        for (int i = 1; i <= 10; ++i) {
            const Vec2 p = from + (to - from) * (static_cast<float>(i) / 10.0f);
            mouse(SDL_EVENT_MOUSE_MOTION, p, p - last);
            last = p;
        }
        mouse(SDL_EVENT_MOUSE_BUTTON_UP, to);
    }

    void ticks(int n)
    {
        for (int i = 0; i < n; ++i)
            app.tick();
    }

    // centre of a message box button, computed with the same layout function as the app
    Vec2 dialogButton(const std::string& text, const std::vector<std::string>& buttons, int index)
    {
        const auto l = ui::layoutMessageBox(*font.metrics(), text, buttons, kScreen,
                                            textures.get("boxBackground").size());
        return l.buttons[static_cast<size_t>(index)].center();
    }

    Vec2 toolButton(ui::EditorButton b)
    {
        ui::EditorToolbar toolbar;
        toolbar.layout(kScreen);
        for (float y = 0; y < kScreen.y; y += 4)
            for (float x : {30.0f, kScreen.x - 30.0f})
                if (toolbar.hit({x, y}) == b)
                    return {x, y + 10};
        return {};
    }

    void screenshot(const char* name)
    {
        app.draw(1.0f, false);
        if (SDL_Surface* s = SDL_RenderReadPixels(sdl, nullptr)) {
            SDL_SavePNG(s, name);
            SDL_DestroySurface(s);
        }
        SDL_RenderPresent(sdl);
    }
};

} // namespace

int main(int, char**)
{
    // fresh user data folder, so the test doesn't see or touch real user levels
    const std::string userDir = std::string(SDL_GetBasePath()) + "smoke_user";
    SDL_RemovePath((userDir + "/levels/level1.json").c_str());
    SDL_setenv_unsafe("SHEEPDOG_USER_DIR", userDir.c_str(), 1);

    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
    SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::printf("SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window* window = nullptr;
    SDL_Renderer* sdl = nullptr;
    if (!SDL_CreateWindowAndRenderer("smoke", static_cast<int>(kScreen.x), static_cast<int>(kScreen.y), 0, &window,
                                     &sdl)) {
        std::printf("window: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetRenderLogicalPresentation(sdl, static_cast<int>(kScreen.x), static_cast<int>(kScreen.y),
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    {
        gfx::Renderer renderer(sdl);
        gfx::TextureCache textures(sdl, gfx::ImageScale::X1);
        gfx::Font font = gfx::Font::load(sdl, gfx::ImageScale::X1);
        CHECK(font.valid());
        auto audio = audio::Audio::open();
        CHECK(audio != nullptr);
        GameApp app(renderer, textures, audio.get());
        // all sound files load and convert
        CHECK(audio && audio->soundsLoaded() == 21 - 2); // gameOver and dogDead are unused
        Harness h{sdl, renderer, textures, font, app};

        // --- menu: no own levels yet, so "play" and "editor"
        h.screenshot("smoke_1_menu.png");
        h.click(h.dialogButton("", {"play", "editor"}, 1));

        // --- editor: a fenced pen with an exit, the dog, two sheep, a bush, a tree and water
        const float x0 = ui::EditorToolbar::columnWidth();
        auto at = [&](float x, float y) { return Vec2(x0 + x, y); };
        h.click(h.toolButton(ui::EditorButton::Fence));
        h.dragLine(at(100, 60), at(600, 60));
        h.dragLine(at(600, 60), at(600, 420));
        h.dragLine(at(600, 420), at(100, 420));
        h.dragLine(at(100, 420), at(100, 60));
        h.click(h.toolButton(ui::EditorButton::Exit));
        h.click(at(520, 240));
        h.click(h.toolButton(ui::EditorButton::Dog));
        h.click(at(200, 240));
        h.click(h.toolButton(ui::EditorButton::Sheep));
        h.click(at(350, 150));
        h.click(at(350, 330));
        h.click(h.toolButton(ui::EditorButton::Bush));
        h.click(at(250, 350));
        h.click(h.toolButton(ui::EditorButton::Tree));
        h.click(at(420, 240));
        h.click(h.toolButton(ui::EditorButton::Water));
        h.dragLine(at(150, 100), at(240, 170));
        // a sheep placed and erased again
        h.click(h.toolButton(ui::EditorButton::Sheep));
        h.click(at(500, 380));
        h.click(h.toolButton(ui::EditorButton::Eraser));
        h.click(at(500, 380));
        h.screenshot("smoke_2_editor.png");

        // --- save: written as own level 1, a valid level
        h.click(h.toolButton(ui::EditorButton::Save));
        h.screenshot("smoke_3_saved.png");
        h.click(h.dialogButton("saved as own level 1", {"ok"}, 0));
        const auto saved = platform::readFile(userDir + "/levels/level1.json");
        CHECK(saved.has_value());
        const auto level = saved ? game::parseLevel(*saved) : std::nullopt;
        CHECK(level.has_value());
        if (level) {
            CHECK(level->fences.size() == 4 && level->exits.size() == 1 && level->dogs.size() == 1);
            CHECK(level->sheep.size() == 2 && level->bushes.size() == 1 && level->trees.size() == 1);
            CHECK(level->water.size() == 1 && level->timeSeconds == 120);
            CHECK(level->width > 500 && level->height > 360);
        }

        // --- test play from the editor, then stop via the pause screen: back in the editor
        h.click(h.toolButton(ui::EditorButton::Play));
        const size_t soundsBefore = audio ? audio->playCount() : 0;
        h.ticks(3 * 24);
        h.screenshot("smoke_4_play.png");
        // bark with the space key: a bark sound is played immediately
        SDL_Event key{};
        key.type = SDL_EVENT_KEY_DOWN;
        key.key.key = SDLK_SPACE;
        h.send(key);
        CHECK(audio && audio->playCount() == soundsBefore + 1);
        h.ticks(3); // bark wave on its way out
        h.screenshot("smoke_4b_bark_wave.png");
        ui::Hud hud;
        hud.layout(kScreen);
        h.click({50, 50}); // pause button
        Vec2 stop;
        for (float x = ui::Hud::width(); x < kScreen.x; x += 5)
            if (hud.hit({x, 240}, true) == ui::HudButton::Stop) {
                stop = {x + 10, 240};
                break;
            }
        h.screenshot("smoke_5_paused.png");
        h.click(stop);
        h.screenshot("smoke_6_back_in_editor.png");

        // --- leave the editor (saved, so no question): the menu now offers own levels
        h.click(h.toolButton(ui::EditorButton::Leave));
        h.screenshot("smoke_7_menu_own_levels.png");
        CHECK(!app.wantsQuit());
    }
    SDL_DestroyRenderer(sdl);
    SDL_DestroyWindow(window);
    SDL_Quit();

    if (gFailures == 0)
        std::printf("app smoke test passed\n");
    return gFailures == 0 ? 0 : 1;
}
