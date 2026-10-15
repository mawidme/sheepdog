# Sheepdog

Herd the sheep into the exits before the time runs out: walk the dog around them and bark to drive them along.
Bonus boxes give you more time, a stronger bark, sleeping or speedy sheep, or make the dog drunk. Comes with a
level editor (desktop).

C++20 with [SDL3](https://libsdl.org) for windowing, input, rendering and audio. Runs on Windows (and other desktops),
Android, the web (Emscripten) and iOS (not tested yet).

## Getting the code

SDL is a git submodule pinned to a release tag:

```
git clone --recurse-submodules <repo>     # or, in an existing clone: git submodule update --init
```

## Building

### Desktop

CMake 3.22+ and a C++20 compiler (e.g. Visual Studio 2022+, which also brings CMake and Ninja).

```
cmake -S . -B build/win
cmake --build build/win --config Release      # build/win/Release/sheepdog.exe (assets are copied next to it)
ctest --test-dir build/win -C Release         # unit tests and the app smoke test
```

### Web (Emscripten)

Install emsdk once (outside the repo, ~1 GB):

```
git clone https://github.com/emscripten-core/emsdk.git %USERPROFILE%\emsdk
cd %USERPROFILE%\emsdk
emsdk install latest
emsdk activate latest
```

Build in a shell where `emsdk_env` was run (needs Ninja, e.g. the one bundled with Visual Studio):

```
emcmake cmake -S . -B build/web -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web
python -m http.server -d build/web 8000     # then open http://localhost:8000/sheepdog.html
```

Deploy `sheepdog.html`, `.js`, `.wasm` and `.data` together; the page must be served over http(s). The web build
packages only the 2x images to keep the download small (about 4 MB). Browsers start sound only after the first
click or key press. The level editor is not available on the web.

### Android

Android SDK with NDK 28.2.13676358 (Gradle downloads it if missing) and CMake 3.22.1 (SDK Manager), and JDK 17 (Android Studio's bundled JBR works).
Gradle builds the native code through the top-level `CMakeLists.txt`; SDL's Java glue is compiled straight from
`external/SDL/android-project`. Or simply open the repo root in Android Studio.

```
gradlew assembleDebug          # app/build/outputs/apk/debug/app-debug.apk
gradlew installDebug           # build and install on a running emulator / device
gradlew assembleRelease        # release APK (signed if keystore.properties exists, see below)
gradlew bundleRelease          # app bundle (.aab) for Google Play
```

`local.properties` (not in git) points to the SDK; Android Studio creates it, or write `sdk.dir=...` yourself.

**Release signing.** Create a key once and keep it safe (losing it means you can't update a published app):

```
keytool -genkeypair -v -keystore sheepdog-release.jks -alias sheepdog -keyalg RSA -keysize 2048 -validity 10000
```

Then create `keystore.properties` in the repo root (both files are ignored by git):

```
storeFile=sheepdog-release.jks
storePassword=...
keyAlias=sheepdog
keyPassword=...
```

### iOS

A Mac with Xcode and CMake. Generate an Xcode project:

```
cmake -S . -B build/ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0 -DSHEEPDOG_IOS_TEAM_ID=<team id>
open build/ios/sheepdog.xcodeproj
```

Select the `sheepdog` scheme and a device or simulator, then run. The team ID is shown in Xcode under
Settings > Accounts. With a free Apple ID (personal team) installs on your own device expire after 7 days, and the
bundle ID may need to be unique: `-DSHEEPDOG_BUNDLE_ID=ch.erglim.sheepdog.<something>`. TestFlight and the App Store
require a paid Apple Developer Program membership.

## Playing

- Mouse / touch: tap to walk there, drag to steer, tap the dog or the bark button to bark, two fingers to zoom
- Keyboard: arrows / WASD walk, space barks, P or Esc pauses (Android: back)
- Mouse wheel zooms, M mutes the sound, F3 shows collision areas
- Own levels (made with the editor) live in the per-user data folder: `SDL_GetPrefPath("erglim", "sheepdog")/levels/`,
  e.g. `%APPDATA%\erglim\sheepdog\levels\levelN.json` on Windows

**Level editor** (desktop, from the main menu): tools on the left place fences, bushes, trees, water, exits, the dog
and sheep; on the right are undo, eraser, pan, load, save, test play and leave. Drag to draw fences and water or to
move what you just placed; the right or middle mouse button pans with any tool, Ctrl+Z undoes. A level needs a dog,
at least one sheep and an exit.

## Code layout

| Folder | Contents |
|---|---|
| `src/core` | vectors, rects, colours, strings, random numbers; no SDL |
| `src/game` | game logic and data, no SDL: levels (JSON), simulation (`World`), level rules (`LevelPlay`, bonuses), editor logic |
| `src/gfx` | 2D rendering on `SDL_Renderer`: textures, sprites, shapes, camera, bitmap font |
| `src/audio` | sound effects: software mixer on an SDL audio stream |
| `src/ui` | menus, message boxes, info bar, level chooser, editor toolbar |
| `src/platform` | asset and user file access per platform |
| `src/game_app.*`, `src/main.cpp` | game flow, input, SDL main callbacks |
| `tests/` | unit tests per module, and `app_smoke` running the whole app offscreen with scripted input |
| `assets/` | everything the game loads at runtime (packaged per platform) |
| `img/`, `tools/` | source art and the tools that bake it into `assets/` |
| `app/`, `cmake/` | Android project; iOS, Windows and web packaging files |

The simulation runs at a fixed 24 ticks per second like the original game; drawing interpolates between ticks.

## Assets

Generated files are committed, so building needs none of these tools. Re-run them after changing the sources.

- **Images**: SVGs in `img/` are baked with [resvg](https://github.com/linebender/resvg) to PNGs at 1x, 2x and 4x in
  `assets/img/<scale>x/`; the game picks the scale matching the display at startup.
  `cmake -P tools/bake_images.cmake` (finds resvg on PATH or in `%USERPROFILE%\tools\resvg`; or pass
  `-DRESVG=<path>`). On Windows unzip `resvg-win64.zip` from the GitHub releases (v0.47.0 is the latest with a
  Windows build); on macOS `brew install resvg`. Some SVGs use the "Aharoni" font; resvg falls back to a default font
  if it is missing.
- **App icons**: `python tools/bake_icons.py` renders `img/icon.svg` for Android (adaptive icon), iOS (asset catalog),
  Windows (.ico) and the desktop window icon. Needs resvg and Pillow (`pip install pillow`).
- **Font**: `python tools/bake_font.py` bakes `tools/fonts/DejaVuSans-Bold.ttf` into bitmap atlases in
  `assets/fonts/<scale>x/` (licence: `assets/fonts/LICENSE-DejaVu.txt`). Needs Pillow.
- **Levels**: JSON files in `assets/levels/levelN.json`, format documented in `src/game/level_data.hpp`.
  `level0` is the main menu background. Levels from the old Qt version (XML) can be converted with
  `python tools/convert_levels_xml_to_json.py old/*.xml --out <folder>`.
- **Sounds**: WAV files in `assets/sfx/` (any PCM format; converted when loaded).
