#pragma once

#include "core/color.hpp"
#include "core/rect.hpp"
#include "texture.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct SDL_Renderer;

// Bitmap font baked by tools/bake_font.py (assets/fonts/<scale>x/ui.png + ui.json).
namespace gfx {

class Renderer;
enum class ImageScale;

enum class TextAlign { Left, Center, Right };

struct GlyphQuad {
    core::Rect source; // in the atlas, logical units (texels / scale) as Renderer::drawTexture expects
    core::Rect dest;
};

// Glyph metrics and text layout; no SDL, so it can be unit tested.
class FontMetrics {
public:
    // nullopt on malformed JSON. pixelsPerUnit: scale the atlas was baked at.
    static std::optional<FontMetrics> parse(std::string_view json, float pixelsPerUnit);

    float lineHeight(float size) const { return (m_ascent + m_descent) * size / m_baseSize; }
    // size of the text block; '\n' starts a new line
    core::Vec2 measure(std::string_view text, float size) const;
    // Quads for each visible glyph. pos is the top-left (Left), top-centre (Center) or
    // top-right (Right) of the block; lines are aligned individually.
    void layout(std::string_view text, core::Vec2 pos, float size, TextAlign align, std::vector<GlyphQuad>& out) const;

private:
    struct Glyph {
        core::Rect source; // logical units
        core::Vec2 offset; // from the pen position on the baseline, at base size
        float advance = 0.0f;
    };
    const Glyph* glyph(char c) const;
    float lineWidth(std::string_view line, float size) const;

    float m_baseSize = 1.0f;
    float m_ascent = 0.0f;
    float m_descent = 0.0f;
    std::unordered_map<char, Glyph> m_glyphs;
};

class Font {
public:
    // empty font (draws nothing) if the files are missing
    static Font load(SDL_Renderer* renderer, ImageScale scale);

    bool valid() const { return m_metrics.has_value() && m_texture.valid(); }
    const FontMetrics* metrics() const { return m_metrics ? &*m_metrics : nullptr; }

    core::Vec2 measure(std::string_view text, float size) const;
    float lineHeight(float size) const { return m_metrics ? m_metrics->lineHeight(size) : size; }

    void draw(Renderer& r, std::string_view text, core::Vec2 pos, float size, core::Color color,
              TextAlign align = TextAlign::Left) const;
    // centred in rect, horizontally and vertically
    void drawCentered(Renderer& r, std::string_view text, const core::Rect& rect, float size, core::Color color) const;

private:
    std::optional<FontMetrics> m_metrics;
    Texture m_texture;
    mutable std::vector<GlyphQuad> m_quads;
};

} // namespace gfx
