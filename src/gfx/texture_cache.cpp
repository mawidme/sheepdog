#include "texture_cache.hpp"

#include "core/strings.hpp"
#include "platform/assets.hpp"

#include <SDL3/SDL.h>

namespace gfx {

ImageScale chooseImageScale(float pixelsPerUnit)
{
    // small tolerance so e.g. 2.05 (rounding of odd screen sizes) still uses 2x
    if (pixelsPerUnit <= 1.05f)
        return ImageScale::X1;
    if (pixelsPerUnit <= 2.05f)
        return ImageScale::X2;
    return ImageScale::X4;
}

TextureCache::TextureCache(SDL_Renderer* renderer, ImageScale scale) : m_renderer(renderer), m_scale(scale) {}

const Texture& TextureCache::get(std::string_view name)
{
    if (auto it = m_textures.find(name); it != m_textures.end())
        return it->second;
    std::string key(name);
    Texture tex = load(key);
    return m_textures.emplace(std::move(key), std::move(tex)).first->second;
}

Texture TextureCache::load(const std::string& name)
{
    const int scale = static_cast<int>(m_scale);
    const std::string path = platform::assetPath(core::format("img/%dx/%s.png", scale, name.c_str()));

    SDL_Surface* surface = SDL_LoadPNG(path.c_str());
    if (!surface) {
        SDL_Log("image '%s' not found (%s): %s", name.c_str(), path.c_str(), SDL_GetError());
        return placeholder();
    }
    Texture tex = Texture::fromSurface(m_renderer, surface, static_cast<float>(scale));
    SDL_DestroySurface(surface);
    if (!tex.valid()) {
        SDL_Log("image '%s': texture creation failed: %s", name.c_str(), SDL_GetError());
        return placeholder();
    }
    return tex;
}

Texture TextureCache::placeholder()
{
    // 32x32 logical units, loud enough to notice
    SDL_Surface* s = SDL_CreateSurface(32, 32, SDL_PIXELFORMAT_RGBA32);
    if (!s)
        return {};
    SDL_FillSurfaceRect(s, nullptr, SDL_MapSurfaceRGBA(s, 255, 0, 255, 255));
    Texture tex = Texture::fromSurface(m_renderer, s);
    SDL_DestroySurface(s);
    return tex;
}

} // namespace gfx
