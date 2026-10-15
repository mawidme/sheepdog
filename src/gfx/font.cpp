#include "font.hpp"

#include "core/strings.hpp"
#include "platform/assets.hpp"
#include "renderer.hpp"
#include "texture_cache.hpp"

#include <SDL3/SDL.h>
#include <cJSON.h>

#include <algorithm>
#include <memory>

using core::Rect;
using core::Vec2;

namespace gfx {

std::optional<FontMetrics> FontMetrics::parse(std::string_view json, float pixelsPerUnit)
{
    std::unique_ptr<cJSON, decltype(&cJSON_Delete)> root(cJSON_ParseWithLength(json.data(), json.size()),
                                                         &cJSON_Delete);
    if (!root)
        return std::nullopt;
    const cJSON* size = cJSON_GetObjectItemCaseSensitive(root.get(), "size");
    const cJSON* ascent = cJSON_GetObjectItemCaseSensitive(root.get(), "ascent");
    const cJSON* descent = cJSON_GetObjectItemCaseSensitive(root.get(), "descent");
    const cJSON* glyphs = cJSON_GetObjectItemCaseSensitive(root.get(), "glyphs");
    if (!cJSON_IsNumber(size) || !cJSON_IsNumber(ascent) || !cJSON_IsNumber(descent) || !cJSON_IsObject(glyphs))
        return std::nullopt;

    FontMetrics m;
    m.m_baseSize = static_cast<float>(size->valuedouble);
    m.m_ascent = static_cast<float>(ascent->valuedouble);
    m.m_descent = static_cast<float>(descent->valuedouble);
    if (m.m_baseSize <= 0.0f)
        return std::nullopt;

    const cJSON* g = nullptr;
    cJSON_ArrayForEach(g, glyphs) {
        const auto code = core::parseInt(g->string ? g->string : "");
        const cJSON* rect = cJSON_GetObjectItemCaseSensitive(g, "rect");
        const cJSON* offset = cJSON_GetObjectItemCaseSensitive(g, "offset");
        const cJSON* advance = cJSON_GetObjectItemCaseSensitive(g, "advance");
        if (!code || *code < 0 || *code > 127 || cJSON_GetArraySize(rect) != 4 || cJSON_GetArraySize(offset) != 2 ||
            !cJSON_IsNumber(advance))
            return std::nullopt;
        auto num = [](const cJSON* arr, int i) { return static_cast<float>(cJSON_GetArrayItem(arr, i)->valuedouble); };
        Glyph glyph;
        glyph.source = {num(rect, 0) / pixelsPerUnit, num(rect, 1) / pixelsPerUnit, num(rect, 2) / pixelsPerUnit,
                        num(rect, 3) / pixelsPerUnit};
        glyph.offset = {num(offset, 0), num(offset, 1)};
        glyph.advance = static_cast<float>(advance->valuedouble);
        m.m_glyphs[static_cast<char>(*code)] = glyph;
    }
    return m;
}

const FontMetrics::Glyph* FontMetrics::glyph(char c) const
{
    auto it = m_glyphs.find(c);
    if (it == m_glyphs.end())
        it = m_glyphs.find('?');
    return it == m_glyphs.end() ? nullptr : &it->second;
}

float FontMetrics::lineWidth(std::string_view line, float size) const
{
    float w = 0.0f;
    for (char c : line)
        if (const Glyph* g = glyph(c))
            w += g->advance;
    return w * size / m_baseSize;
}

Vec2 FontMetrics::measure(std::string_view text, float size) const
{
    float width = 0.0f;
    int lines = 0;
    for (const std::string& line : core::split(text, '\n')) {
        width = std::max(width, lineWidth(line, size));
        ++lines;
    }
    return {width, lineHeight(size) * static_cast<float>(lines)};
}

void FontMetrics::layout(std::string_view text, Vec2 pos, float size, TextAlign align, std::vector<GlyphQuad>& out) const
{
    const float scale = size / m_baseSize;
    float y = pos.y;
    for (const std::string& line : core::split(text, '\n')) {
        const float w = lineWidth(line, size);
        float x = pos.x;
        if (align == TextAlign::Center)
            x -= w / 2.0f;
        else if (align == TextAlign::Right)
            x -= w;
        const float baseline = y + m_ascent * scale;
        for (char c : line) {
            const Glyph* g = glyph(c);
            if (!g)
                continue;
            if (!g->source.isEmpty()) {
                const Vec2 topLeft{x + g->offset.x * scale, baseline + g->offset.y * scale};
                out.push_back({g->source, {topLeft, g->source.size() * (size / m_baseSize)}});
            }
            x += g->advance * scale;
        }
        y += lineHeight(size);
    }
}

Font Font::load(SDL_Renderer* renderer, ImageScale scale)
{
    Font font;
    const int s = static_cast<int>(scale);
    const auto json = platform::readAssetFile(core::format("fonts/%dx/ui.json", s));
    if (json)
        font.m_metrics = FontMetrics::parse(*json, static_cast<float>(s));
    if (!font.m_metrics)
        SDL_Log("font metrics missing or invalid");

    const std::string png = platform::assetPath(core::format("fonts/%dx/ui.png", s));
    if (SDL_Surface* surface = SDL_LoadPNG(png.c_str())) {
        font.m_texture = Texture::fromSurface(renderer, surface, static_cast<float>(s));
        SDL_DestroySurface(surface);
    } else {
        SDL_Log("font atlas %s: %s", png.c_str(), SDL_GetError());
    }
    return font;
}

Vec2 Font::measure(std::string_view text, float size) const
{
    return m_metrics ? m_metrics->measure(text, size) : Vec2();
}

void Font::draw(Renderer& r, std::string_view text, Vec2 pos, float size, core::Color color, TextAlign align) const
{
    if (!valid())
        return;
    m_quads.clear();
    m_metrics->layout(text, pos, size, align, m_quads);
    const float opacity = static_cast<float>(color.a) / 255.0f;
    for (const GlyphQuad& q : m_quads)
        r.drawTexture(m_texture, q.dest, {.opacity = opacity, .tint = color.withAlpha(255), .source = &q.source});
}

void Font::drawCentered(Renderer& r, std::string_view text, const Rect& rect, float size, core::Color color) const
{
    const Vec2 sz = measure(text, size);
    draw(r, text, {rect.center().x, rect.center().y - sz.y / 2.0f}, size, color, TextAlign::Center);
}

} // namespace gfx
