#include "renderer.hpp"

#include "camera.hpp"
#include "tessellate.hpp"
#include "texture.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

using core::Color;
using core::Rect;
using core::Vec2;

namespace gfx {

namespace {

SDL_FRect toSdl(const Rect& r) { return {r.x, r.y, r.w, r.h}; }

SDL_FColor toSdl(Color c)
{
    return {c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f};
}

} // namespace

Renderer::Renderer(SDL_Renderer* renderer) : m_renderer(renderer)
{
    // alpha blending for shapes (SDL_RenderFillRect, SDL_RenderGeometry without texture)
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
}

Vec2 Renderer::screenSize() const
{
    int w = 0, h = 0;
    SDL_RendererLogicalPresentation mode;
    if (SDL_GetRenderLogicalPresentation(m_renderer, &w, &h, &mode) && mode != SDL_LOGICAL_PRESENTATION_DISABLED)
        return {static_cast<float>(w), static_cast<float>(h)};
    SDL_GetCurrentRenderOutputSize(m_renderer, &w, &h);
    return {static_cast<float>(w), static_cast<float>(h)};
}

void Renderer::clear(Color color)
{
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_renderer);
}

void Renderer::present()
{
    SDL_RenderPresent(m_renderer);
}

void Renderer::setClipRect(const Rect* screenRect)
{
    if (!screenRect) {
        SDL_SetRenderClipRect(m_renderer, nullptr);
        return;
    }
    const SDL_Rect r{static_cast<int>(std::floor(screenRect->x)), static_cast<int>(std::floor(screenRect->y)),
                     static_cast<int>(std::ceil(screenRect->w)), static_cast<int>(std::ceil(screenRect->h))};
    SDL_SetRenderClipRect(m_renderer, &r);
}

Vec2 Renderer::toScreen(Vec2 p) const { return m_camera ? m_camera->worldToScreen(p) : p; }
Rect Renderer::toScreen(const Rect& r) const { return m_camera ? m_camera->worldToScreen(r) : r; }
float Renderer::toScreen(float length) const { return m_camera ? length * m_camera->zoom : length; }

void Renderer::drawTexture(const Texture& texture, const Rect& dst, const SpriteParams& params)
{
    if (!texture.valid() || params.opacity <= 0.0f)
        return;

    SDL_Texture* tex = texture.sdl();
    SDL_SetTextureAlphaModFloat(tex, params.opacity);
    SDL_SetTextureColorModFloat(tex, params.tint.r / 255.0f, params.tint.g / 255.0f, params.tint.b / 255.0f);

    const SDL_FRect d = toSdl(toScreen(dst));
    SDL_FRect src;
    if (params.source) {
        const float ppu = texture.pixelsPerUnit();
        src = {params.source->x * ppu, params.source->y * ppu, params.source->w * ppu, params.source->h * ppu};
    }

    if (params.rotationDeg == 0.0f && !params.flipX) {
        SDL_RenderTexture(m_renderer, tex, params.source ? &src : nullptr, &d);
    } else {
        const SDL_FPoint center{d.w * params.pivot.x, d.h * params.pivot.y};
        SDL_RenderTextureRotated(m_renderer, tex, params.source ? &src : nullptr, &d, params.rotationDeg, &center,
                                 params.flipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }
}

void Renderer::fillRect(const Rect& r, Color color)
{
    const SDL_FRect d = toSdl(toScreen(r));
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_renderer, &d);
}

void Renderer::drawRect(const Rect& r, Color color, float width)
{
    tess::rectOutline(m_triangles, toScreen(r), toScreen(width));
    flushTriangles(color);
}

void Renderer::fillEllipse(const Rect& bounds, Color color)
{
    const Rect s = toScreen(bounds);
    tess::ellipse(m_triangles, s, tess::circleSegments(std::max(s.w, s.h) * 0.5f));
    flushTriangles(color);
}

void Renderer::drawEllipse(const Rect& bounds, Color color, float width)
{
    const Rect s = toScreen(bounds);
    tess::ellipseOutline(m_triangles, s, toScreen(width), tess::circleSegments(std::max(s.w, s.h) * 0.5f));
    flushTriangles(color);
}

void Renderer::fillRoundedRect(const Rect& r, float radius, Color color)
{
    const float screenRadius = toScreen(radius);
    tess::roundedRect(m_triangles, toScreen(r), screenRadius,
                      std::max(2, tess::circleSegments(screenRadius) / 4));
    flushTriangles(color);
}

void Renderer::drawLine(Vec2 a, Vec2 b, Color color, float width)
{
    tess::line(m_triangles, toScreen(a), toScreen(b), toScreen(width));
    flushTriangles(color);
}

void Renderer::flushTriangles(Color color)
{
    if (m_triangles.empty())
        return;
    const SDL_FColor c = toSdl(color);
    m_vertices.clear();
    m_vertices.reserve(m_triangles.size());
    for (const Vec2& p : m_triangles)
        m_vertices.push_back({{p.x, p.y}, c, {0.0f, 0.0f}});
    SDL_RenderGeometry(m_renderer, nullptr, m_vertices.data(), static_cast<int>(m_vertices.size()), nullptr, 0);
    m_triangles.clear();
}

} // namespace gfx
