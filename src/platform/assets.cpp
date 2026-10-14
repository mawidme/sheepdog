#include "assets.hpp"

#include <SDL3/SDL.h>

namespace platform {

std::string assetPath(std::string_view relativePath)
{
#ifdef SDL_PLATFORM_ANDROID
    // relative paths are looked up in the APK's assets, whose root is our assets/ folder
    return std::string(relativePath);
#else
    const char* base = SDL_GetBasePath(); // owned by SDL, ends with a separator
    std::string path = base ? base : "";
    path += "assets/";
    path += relativePath;
    return path;
#endif
}

} // namespace platform
