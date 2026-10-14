#pragma once

#include "texture.hpp"

#include <map>
#include <string>
#include <string_view>

struct SDL_Renderer;

namespace gfx {

// Resolutions the SVGs are baked at (tools/bake_images.cmake), as texels per logical unit.
enum class ImageScale { X1 = 1, X2 = 2, X4 = 4 };

// Smallest baked scale that is at least as sharp as the display needs.
// pixelsPerUnit: physical pixels per logical canvas unit (e.g. 1080 px / 480 units = 2.25).
ImageScale chooseImageScale(float pixelsPerUnit);

// Loads the game's images on first use and keeps them for the cache's lifetime.
class TextureCache {
public:
    TextureCache(SDL_Renderer* renderer, ImageScale scale);

    // name as in img/ without extension, e.g. "sheep01" or "bonus/box".
    // A missing image is logged once and replaced by a magenta placeholder.
    const Texture& get(std::string_view name);

    ImageScale scale() const { return m_scale; }

private:
    Texture load(const std::string& name);
    Texture placeholder();

    SDL_Renderer*                            m_renderer;
    ImageScale                               m_scale;
    std::map<std::string, Texture, std::less<>> m_textures;
};

} // namespace gfx
