#pragma once

#include <cstdint>

namespace core {

// 8-bit RGBA colour. Replaces QColor.
struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    constexpr Color() = default;
    constexpr Color(std::uint8_t r_, std::uint8_t g_, std::uint8_t b_, std::uint8_t a_ = 255)
        : r(r_), g(g_), b(b_), a(a_) {}

    // 0xRRGGBB, fully opaque
    static constexpr Color fromRgb(std::uint32_t rgb)
    {
        return {static_cast<std::uint8_t>(rgb >> 16), static_cast<std::uint8_t>(rgb >> 8),
                static_cast<std::uint8_t>(rgb)};
    }

    constexpr bool operator==(const Color&) const = default;

    constexpr Color withAlpha(std::uint8_t alpha) const { return {r, g, b, alpha}; }

    static constexpr Color black() { return {0, 0, 0}; }
    static constexpr Color white() { return {255, 255, 255}; }
    static constexpr Color transparent() { return {0, 0, 0, 0}; }
};

} // namespace core
