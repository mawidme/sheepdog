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
    // tests point this to a temporary folder
    if (const char* dir = SDL_getenv("SHEEPDOG_USER_DIR"))
        return std::string(dir) + "/";
#ifdef SDL_PLATFORM_IOS
    // the app's Documents folder: shown in the Files app (UIFileSharingEnabled), so levels can be shared
    if (const char* documents = SDL_GetUserFolder(SDL_FOLDER_DOCUMENTS))
        return documents;
#endif
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

bool writeFile(const std::string& path, std::string_view content)
{
    const size_t slash = path.find_last_of("/\\");
    if (slash != std::string::npos && !SDL_CreateDirectory(path.substr(0, slash).c_str())) {
        SDL_Log("can't create folder for '%s': %s", path.c_str(), SDL_GetError());
        return false;
    }
    if (!SDL_SaveFile(path.c_str(), content.data(), content.size())) {
        SDL_Log("can't write '%s': %s", path.c_str(), SDL_GetError());
        return false;
    }
    return true;
}

} // namespace platform
