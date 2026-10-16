#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace core {

// printf-style formatting into a std::string. Replaces QString::arg().
// (std::format is not available in all target toolchains, e.g. NDK 25 libc++.)
#if defined(__GNUC__) || defined(__clang__)
std::string format(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
#else
std::string format(const char* fmt, ...);
#endif

std::vector<std::string> split(std::string_view s, char sep, bool skipEmpty = false);
std::string_view trim(std::string_view s);
std::string toLower(std::string_view s);

// whole string must be a number (surrounding whitespace allowed)
std::optional<int> parseInt(std::string_view s);
std::optional<float> parseFloat(std::string_view s);

} // namespace core
