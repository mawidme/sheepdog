// Entry point. Uses SDL3's main-callbacks model, which is the portable way to
// drive the loop on iOS and Emscripten (no blocking while-loop in main()).
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

namespace {

constexpr int    kWindowWidth  = 800;
constexpr int    kWindowHeight = 480;
constexpr double kFixedStep    = 1.0 / 60.0;

struct App {
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    Uint64        lastTicks = 0;
    double        accumulator = 0.0;
    double        time = 0.0;
};

void update(App& app, double dt)
{
    app.time += dt;
}

void render(App& app)
{
    // placeholder: pulse the clear colour so a running loop is visible
    const float g = 0.35f + 0.1f * static_cast<float>(SDL_sin(app.time * 2.0));
    SDL_SetRenderDrawColorFloat(app.renderer, 0.2f, g, 0.2f, 1.0f);
    SDL_RenderClear(app.renderer);
    SDL_RenderPresent(app.renderer);
}

} // namespace

SDL_AppResult SDL_AppInit(void** appstate, int /*argc*/, char** /*argv*/)
{
    SDL_SetAppMetadata("Sheepdog", "0.1", "ch.erglim.sheepdog");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    auto* app = new App;
    if (!SDL_CreateWindowAndRenderer("Sheepdog", kWindowWidth, kWindowHeight,
                                     SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
                                     &app->window, &app->renderer)) {
        SDL_Log("window/renderer creation failed: %s", SDL_GetError());
        delete app;
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderVSync(app->renderer, 1);
    SDL_SetRenderLogicalPresentation(app->renderer, kWindowWidth, kWindowHeight,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    app->lastTicks = SDL_GetTicksNS();
    *appstate = app;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* /*appstate*/, SDL_Event* event)
{
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        if (event->key.key == SDLK_ESCAPE)
            return SDL_APP_SUCCESS;
        break;
    default:
        break;
    }
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
        update(app, kFixedStep);
        app.accumulator -= kFixedStep;
    }

    render(app);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/)
{
    auto* app = static_cast<App*>(appstate);
    if (app) {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }
}
