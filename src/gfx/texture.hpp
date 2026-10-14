#pragma once

#include "core/vec2.hpp"

struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;

namespace gfx {

// Owning wrapper around an SDL_Texture. Move-only.
class Texture {
public:
    Texture() = default;
    ~Texture();
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // empty Texture on failure (see SDL_GetError())
    static Texture fromSurface(SDL_Renderer* renderer, SDL_Surface* surface);

    bool valid() const { return m_texture != nullptr; }
    SDL_Texture* sdl() const { return m_texture; }
    core::Vec2 size() const { return m_size; }

private:
    SDL_Texture* m_texture = nullptr;
    core::Vec2   m_size;
};

} // namespace gfx
