#include "assets.hpp"

#include <SDL3/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

namespace {

std::function<void(std::string)> gOnPicked;
constexpr const char* kPickedFile = "/tmp/picked";

// a hidden <input type="file">; the picked file is written to kPickedFile, then sheepdog_file_picked() runs
EM_JS(void, jsPickFile, (const char* accept), {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = UTF8ToString(accept);
    input.style.display = 'none';
    input.addEventListener('change', () => {
        const file = input.files[0];
        input.remove();
        if (file)
            file.arrayBuffer().then((data) => {
                FS.writeFile('/tmp/picked', new Uint8Array(data));
                _sheepdog_file_picked();
            });
    });
    input.addEventListener('cancel', () => input.remove());
    document.body.appendChild(input); // iOS Safari opens the dialog only for inputs in the document

    // Safari opens the dialog only from within a user input event handler, but the game handles the press a frame
    // later, from its main loop. So open it when that press (usually still held) is released; after a very quick
    // tap fall back to opening it right away, which other browsers allow.
    const events = ['pointerup', 'mouseup', 'touchend'];
    let opened = false;
    const open = () => {
        if (opened)
            return;
        opened = true;
        events.forEach((type) => document.removeEventListener(type, open, true));
        input.click();
    };
    events.forEach((type) => document.addEventListener(type, open, true));
    setTimeout(open, 500);
});

EM_JS(void, jsDownloadFile, (const char* name, const char* data, int size), {
    const blob = new Blob([HEAPU8.slice(data, data + size)], {type: 'application/octet-stream'});
    const link = document.createElement('a');
    link.href = URL.createObjectURL(blob);
    link.download = UTF8ToString(name);
    document.body.appendChild(link);
    link.click();
    link.remove();
    setTimeout(() => URL.revokeObjectURL(link.href), 10000);
});

} // namespace

extern "C" EMSCRIPTEN_KEEPALIVE void sheepdog_file_picked()
{
    auto content = platform::readFile(kPickedFile);
    SDL_RemovePath(kPickedFile);
    if (content && gOnPicked)
        gOnPicked(std::move(*content));
}
#endif

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

#ifdef __EMSCRIPTEN__
void pickFile(const char* accept, std::function<void(std::string content)> onPicked)
{
    gOnPicked = std::move(onPicked);
    jsPickFile(accept);
}

void downloadFile(const std::string& name, std::string_view content)
{
    jsDownloadFile(name.c_str(), content.data(), static_cast<int>(content.size()));
}
#endif

} // namespace platform
