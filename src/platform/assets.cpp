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

std::optional<std::string> readAssetFile(std::string_view relativePath)
{
    const std::string path = assetPath(relativePath);
    size_t size = 0;
    void* data = SDL_LoadFile(path.c_str(), &size); // also reads APK assets on Android
    if (!data) {
        SDL_Log("can't read '%s': %s", path.c_str(), SDL_GetError());
        return std::nullopt;
    }
    std::string content(static_cast<const char*>(data), size);
    SDL_free(data);
    return content;
}

bool assetExists(std::string_view relativePath)
{
    SDL_IOStream* io = SDL_IOFromFile(assetPath(relativePath).c_str(), "rb");
    if (!io)
        return false;
    SDL_CloseIO(io);
    return true;
}

std::string userDataPath()
{
    char* path = SDL_GetPrefPath("erglim", "sheepdog");
    std::string result = path ? path : "";
    SDL_free(path);
    return result;
}

std::optional<std::string> readFile(const std::string& path)
{
    size_t size = 0;
    void* data = SDL_LoadFile(path.c_str(), &size);
    if (!data)
        return std::nullopt;
    std::string content(static_cast<const char*>(data), size);
    SDL_free(data);
    return content;
}

} // namespace platform
