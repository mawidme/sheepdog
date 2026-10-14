# BUILDING (SDL port, branch refactor/no-qt)

The SDL3 port is built with CMake. SDL is a git submodule pinned to a release tag.

```
git clone --recurse-submodules <repo>     # or: git submodule update --init
cmake -S . -B build/win
cmake --build build/win --config Debug
```

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
