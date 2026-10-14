// Entry point. Uses SDL3's main-callbacks model, which is the portable way to
// drive the loop on iOS and Emscripten (no blocking while-loop in main()).
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "demo.hpp"
#include "gfx/renderer.hpp"

#include <cmath>
#include <memory>

namespace {

// Minimum logical canvas; the game is laid out for 800x480 (as the Qt version).
constexpr int    kMinLogicalWidth  = 800;
constexpr int    kMinLogicalHeight = 480;
constexpr double kFixedStep        = 1.0 / 60.0;

struct App {
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::unique_ptr<gfx::Renderer> gfx;
    std::unique_ptr<Demo>          demo;
    Uint64        lastTicks = 0;
    double        accumulator = 0.0;
};

// Logical canvas covers at least 800x480 and matches the window's aspect ratio,
// so wide phone screens get more visible level instead of black bars.
void updateLogicalSize(App& app)
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(app.window, &w, &h);
    if (w <= 0 || h <= 0)
        return;
    const double aspect = static_cast<double>(w) / h;
    int lw = kMinLogicalWidth, lh = kMinLogicalHeight;
    if (aspect > static_cast<double>(kMinLogicalWidth) / kMinLogicalHeight)
        lw = static_cast<int>(std::lround(lh * aspect));
    else
        lh = static_cast<int>(std::lround(lw / aspect));
    SDL_SetRenderLogicalPresentation(app.renderer, lw, lh, SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

} // namespace

SDL_AppResult SDL_AppInit(void** appstate, int /*argc*/, char** /*argv*/)
{
    SDL_SetAppMetadata("Sheepdog", "0.1", "ch.erglim.sheepdog");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // landscape only on mobile; must be set before the window is created
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS)
    constexpr SDL_WindowFlags kWindowFlags = SDL_WINDOW_FULLSCREEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#else
    constexpr SDL_WindowFlags kWindowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#endif

    auto* app = new App;
    if (!SDL_CreateWindowAndRenderer("Sheepdog", kMinLogicalWidth, kMinLogicalHeight, kWindowFlags,
                                     &app->window, &app->renderer)) {
        SDL_Log("window/renderer creation failed: %s", SDL_GetError());
        delete app;
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderVSync(app->renderer, 1);
    updateLogicalSize(*app);

    app->gfx = std::make_unique<gfx::Renderer>(app->renderer);
    app->demo = std::make_unique<Demo>(*app->gfx);

    app->lastTicks = SDL_GetTicksNS();
    *appstate = app;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    auto& app = *static_cast<App*>(appstate);
    // window/pixel coordinates -> logical canvas coordinates
    SDL_ConvertEventToRenderCoordinates(app.renderer, event);

    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        if (event->key.key == SDLK_ESCAPE)
            return SDL_APP_SUCCESS;
        break;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        updateLogicalSize(app);
        break;
    default:
        break;
    }
    app.demo->handleEvent(*event);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    auto& app = *static_cast<App*>(appstate);

    const Uint64 now = SDL_GetTicksNS();
    double frame = static_cast<double>(now - app.lastTicks) * 1e-9;
    app.lastTicks = now;
    if (frame > 0.25) frame = 0.25; // avoid spiral of death after a stall

    app.accumulator += frame;
    while (app.accumulator >= kFixedStep) {
        app.demo->update(kFixedStep);
        app.accumulator -= kFixedStep;
    }

    app.demo->draw();
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/)
{
    auto* app = static_cast<App*>(appstate);
    if (app) {
        app->demo.reset(); // owns textures, must go before the SDL renderer
        app->gfx.reset();
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }
}
