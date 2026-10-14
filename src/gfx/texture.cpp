#include "texture.hpp"

#include <SDL3/SDL.h>

#include <utility>

namespace gfx {

Texture::~Texture()
{
    if (m_texture)
        SDL_DestroyTexture(m_texture);
}

Texture::Texture(Texture&& other) noexcept
    : m_texture(std::exchange(other.m_texture, nullptr)),
      m_pixelSize(other.m_pixelSize),
      m_pixelsPerUnit(other.m_pixelsPerUnit)
{
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        if (m_texture)
            SDL_DestroyTexture(m_texture);
        m_texture = std::exchange(other.m_texture, nullptr);
        m_pixelSize = other.m_pixelSize;
        m_pixelsPerUnit = other.m_pixelsPerUnit;
    }
    return *this;
}

Texture Texture::fromSurface(SDL_Renderer* renderer, SDL_Surface* surface, float pixelsPerUnit)
{
    Texture tex;
    if (!surface)
        return tex;
    tex.m_texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (tex.m_texture) {
        float w = 0, h = 0;
        SDL_GetTextureSize(tex.m_texture, &w, &h);
        tex.m_pixelSize = {w, h};
        tex.m_pixelsPerUnit = pixelsPerUnit;
    }
    return tex;
}

} // namespace gfx
