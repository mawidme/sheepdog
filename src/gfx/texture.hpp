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

    // Empty Texture on failure (see SDL_GetError()).
    // pixelsPerUnit: how many texels make up one logical unit (the image scale it was baked at).
    static Texture fromSurface(SDL_Renderer* renderer, SDL_Surface* surface, float pixelsPerUnit = 1.0f);

    bool valid() const { return m_texture != nullptr; }
    SDL_Texture* sdl() const { return m_texture; }

    // logical size, i.e. the size of the source SVG; use this for layout
    core::Vec2 size() const { return m_pixelSize / m_pixelsPerUnit; }
    core::Vec2 pixelSize() const { return m_pixelSize; }
    float pixelsPerUnit() const { return m_pixelsPerUnit; }

private:
    SDL_Texture* m_texture = nullptr;
    core::Vec2   m_pixelSize;
    float        m_pixelsPerUnit = 1.0f;
};

} // namespace gfx
