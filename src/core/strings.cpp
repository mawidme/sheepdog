#include "strings.hpp"

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace core {

std::string format(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    va_list argsCopy;
    va_copy(argsCopy, args);
    const int len = std::vsnprintf(nullptr, 0, fmt, argsCopy);
    va_end(argsCopy);

    std::string result;
    if (len > 0) {
        result.resize(static_cast<size_t>(len));
        std::vsnprintf(result.data(), result.size() + 1, fmt, args);
    }
    va_end(args);
    return result;
}

std::vector<std::string> split(std::string_view s, char sep, bool skipEmpty)
{
    std::vector<std::string> parts;
    size_t start = 0;
    while (true) {
        const size_t end = s.find(sep, start);
        const std::string_view part = s.substr(start, end == std::string_view::npos ? end : end - start);
        if (!skipEmpty || !part.empty())
            parts.emplace_back(part);
        if (end == std::string_view::npos)
            break;
        start = end + 1;
    }
    return parts;
}

std::string_view trim(std::string_view s)
{
    auto isSpace = [](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; };
    while (!s.empty() && isSpace(s.front()))
        s.remove_prefix(1);
    while (!s.empty() && isSpace(s.back()))
        s.remove_suffix(1);
    return s;
}

std::string toLower(std::string_view s)
{
    std::string out(s);
    for (char& c : out)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

// strtol/strtof need a null-terminated string; std::from_chars for floats
// is missing in older libc++ versions (Android NDK, iOS < 16.3)
std::optional<int> parseInt(std::string_view s)
{
    const std::string str(trim(s));
    if (str.empty())
        return std::nullopt;
    char* end = nullptr;
    errno = 0;
    const long value = std::strtol(str.c_str(), &end, 10);
    if (errno != 0 || *end != '\0' || value < INT_MIN || value > INT_MAX)
        return std::nullopt;
    return static_cast<int>(value);
}

std::optional<float> parseFloat(std::string_view s)
{
    const std::string str(trim(s));
    if (str.empty())
        return std::nullopt;
    char* end = nullptr;
    errno = 0;
    const float value = std::strtof(str.c_str(), &end);
    if (errno != 0 || *end != '\0')
        return std::nullopt;
    return value;
}

} // namespace core
