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
