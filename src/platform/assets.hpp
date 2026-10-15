#pragma once

#include <optional>
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

// Whole content of an asset file (e.g. "levels/level0.json"); nullopt and a log message if it can't be read.
std::optional<std::string> readAssetFile(std::string_view relativePath);
// true if the asset can be opened (no log message otherwise)
bool assetExists(std::string_view relativePath);

// Writable per-user folder (SDL_GetPrefPath), ends with a separator; user levels go in "levels/".
std::string userDataPath();
// whole content of a file given by its full path; nullopt (no log message) if it can't be read
std::optional<std::string> readFile(const std::string& path);
// writes the file, creating missing folders; false (and a log message) on failure
bool writeFile(const std::string& path, std::string_view content);

} // namespace platform
