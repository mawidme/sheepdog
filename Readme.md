# BUILDING (SDL port, branch refactor/no-qt)

The SDL3 port is built with CMake. SDL is a git submodule pinned to a release tag.

```
git clone --recurse-submodules <repo>     # or: git submodule update --init
cmake -S . -B build/win
cmake --build build/win --config Debug
ctest --test-dir build/win -C Debug        # unit tests (desktop builds only)
```

Code layout: `src/core` holds platform-independent utilities (vectors, rects, colours, strings, random) with no SDL
dependency; `src/gfx` is the 2D rendering layer on top of `SDL_Renderer` (textures, sprites, shapes, camera).
Both are unit tested in `tests/` (gfx tests cover the SDL-free parts: camera maths and shape tessellation).

## Web (Emscripten)

Install emsdk once (outside the repo, ~1 GB):

```
git clone https://github.com/emscripten-core/emsdk.git %USERPROFILE%\emsdk
cd %USERPROFILE%\emsdk
emsdk install latest
emsdk activate latest
```

Build (in a shell where `emsdk_env` was sourced; needs Ninja, e.g. the one bundled with Visual Studio):

```
emcmake cmake -S . -B build/web -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web
python -m http.server -d build/web 8000   # then open http://localhost:8000/sheepdog.html
```

The page must be served over http; opening the .html file directly does not work.

## Android

Requires Android Studio (or the Android SDK) with NDK 25.1.8937393 and CMake 3.22.1 installed via the SDK Manager, and a JDK 17
(Android Studio's bundled JBR works). Gradle builds the native code through the top-level `CMakeLists.txt`; SDL's Java glue
is compiled straight from `external/SDL/android-project`, so there is nothing to copy.

```
gradlew assembleDebug                      # APK in app/build/outputs/apk/debug/
gradlew installDebug                       # build + install on a running emulator/device
```

Or open the repo root in Android Studio. `local.properties` points to the SDK; adjust `sdk.dir` if yours lives elsewhere.

## iOS (untested so far)

Requires a Mac with Xcode and CMake. Generate an Xcode project:

```
cmake -S . -B build/ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0       -DSHEEPDOG_IOS_TEAM_ID=<your team id>
open build/ios/sheepdog.xcodeproj
```

Select the `sheepdog` scheme and a device or simulator, then run. The team ID is shown in Xcode under
Settings > Accounts. With a free Apple ID (personal team), installs on your own device expire after 7 days, and the
bundle ID may need to be made unique: `-DSHEEPDOG_BUNDLE_ID=ch.erglim.sheepdog.<something>`.
TestFlight requires a paid Apple Developer Program membership.

## Images

The SVGs in `img/` are the source art. The game uses PNGs baked from them at three scales (1x, 2x, 4x texels per
logical unit) in `assets/img/<scale>x/`; the game picks the scale matching the display density at startup.
The baked PNGs are committed, so building does not need any SVG tooling.

After adding or changing an SVG, re-bake all PNGs with [resvg](https://github.com/linebender/resvg) and commit them:

```
cmake -P tools/bake_images.cmake                       # finds resvg on PATH or in %USERPROFILE%\tools\resvg
cmake -DRESVG=/path/to/resvg -P tools/bake_images.cmake  # or point to it explicitly
```

resvg: on Windows unzip `resvg-win64.zip` from the GitHub releases (v0.47.0 is the latest with a Windows build) to
`%USERPROFILE%\tools\resvg`; on macOS `brew install resvg`. A few SVGs contain text in the "Aharoni" font; if it is not
installed, resvg falls back to a default font.

## Levels

Built-in levels are JSON files in `assets/levels/levelN.json`; the format is documented in
`src/game/level_data.hpp`. They were converted from the old XML files in `levels/` (still used by the legacy Qt build)
with `python tools/convert_levels_xml_to_json.py levels/*.xml --out assets/levels`, which also works for old user levels.

## Font

UI text uses a bitmap font baked from `tools/fonts/DejaVuSans-Bold.ttf` into `assets/fonts/<scale>x/` (atlas PNG +
metrics JSON, committed; licence in `assets/fonts/LICENSE-DejaVu.txt`). Re-bake after changing font, size or
characters (needs Pillow: `pip install pillow`):

```
python tools/bake_font.py
```

## Controls

- Mouse / touch: tap to walk there, drag to steer, tap the dog or the bark button to bark, two fingers to zoom
- Keyboard: arrows / WASD walk, space barks, P or Esc pauses (Android: back)
- Mouse wheel zooms; F3 shows collision areas
- User levels are read from the per-user data folder (`SDL_GetPrefPath`, e.g. `%APPDATA%\erglim\sheepdog\levels\levelN.json`)

The old Qt build is kept until the port reaches parity: `cmake -DSHEEPDOG_LEGACY_QT=ON ...` (see the Qt notes below).

---

# REQUIREMENTS

* Qt
- download qt 5 sources zip: https://www.qt.io/offline-installers
  - unzip content of qt-everywhere-src-5.xx.x sub-dir to c:\qt\src
- create build dir: `mkdir c:\qt\build`
- configure qt5
  - https://doc.qt.io/qt-5/android-building.html
    - JDK: from Android Studio
    - MinGW: LLVM-MinGW ucrt-x86_64, https://github.com/mstorsjo/llvm-mingw/releases
    - Perl: http://strawberryperl.com/
  - run Configure.ps1 from repo root (adjusting paths first if required)
    - first used: ..\configure.bat -platform win32-g++ -xplatform android-clang -prefix c:\qt -nomake tests -nomake examples -android-sdk $sdkPath -android-ndk $ndkPath -android-ndk-host windows-x86_64 -no-warnings-are-errors
    - in case this fails due to "error: ‘numeric_limits’ is not a class template": add these 3 lines at the top of qtbase/src/corelib/global/qglobal.h:
      #ifdef __cplusplus
      #include <limits>
      #endif
    - in case this fails due to "unknown argument: '-fno-keep-inline-dllexport'": comment argument in qmake.conf resp. Makefile (use file search)
- configure qt6
  - https://wiki.qt.io/Building_Qt_6_from_Git
  - https://doc.qt.io/qt-6.5/android-building.html
  - open console: x64 Native Tools Command Prompt for VS 2022
  - mkdir c:\qt\qt6-windows-build
  - cd c:\qt\qt6-windows-build
  - c:\qt6-src\configure.bat -prefix C:\qt\qt6-windows
  - cmake --build .
  - cmake --install .
  - run ConfigureQt6.ps1 from repo root (adjusting paths first if required)
- cmake --build . --parallel
- build
  - qt5: mingw32-make.exe -j4
  - qt6: cmake --build . --parallel
- install
  - qt5: mingw32-make.exe install
  - qt6: cmake --install .

* Qt Creator open source?
https://download.qt.io/official_releases/qtcreator/12.0/12.0.2/qt-creator-opensource-windows-x86_64-12.0.2.exe.mirrorlist?__hstc=45788219.64f02eb7ed6ba237641c1ba3ab97ff53.1689838104787.1689838104787.1689838104787.1&__hssc=45788219.1.1689838104787&__hsfp=783877104
