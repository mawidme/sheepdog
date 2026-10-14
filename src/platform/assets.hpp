#pragma once

#include <string>
#include <string_view>

namespace platform {

// Full path of a read-only game asset, given its path relative to the repo's assets/ folder
// (e.g. "img/2x/sheep01.png"). Works for every platform's packaging:
//   desktop: assets/ is copied next to the executable
//   Android: assets/ is the APK's asset root (read by SDL_IOFromFile)
//   iOS:     assets/ is copied into the app bundle
//   web:     assets/ is preloaded into the virtual file system at /assets
std::string assetPath(std::string_view relativePath);

} // namespace platform
