@REM $env:MINGW_ROOT = "$env:USERPROFILE\Documents\devel\mingw64-win32-msvcrt\bin"
@REM $env:MINGW_ROOT = "$env:USERPROFILE\Documents\devel\llvm-mingw-20240320-ucrt-x86_64\bin"
set MINGW_ROOT=%USERPROFILE%\Documents\devel\mingw64-win32-ucrt\bin
set NINJA_PATH=%USERPROFILE%\Documents\devel\ninja
set PATH=%MINGW_ROOT%;%PATH%

@REM $makePath = (get-command mingw32-make.exe).source
@REM if ($makePath -ne "$($env:MINGW_ROOT)\mingw32-make.exe") { Read-Host "Unexpected mingw make path '$makePath'. Enter to continue."; exit }

set ANDROID_SDK_ROOT=%USERPROFILE%\AppData\Local\Android\Sdk
set ANDROID_NDK_ROOT=%USERPROFILE%\AppData\Local\Android\Sdk\ndk\25.1.8937393

@REM set ANDROID_ABI=arm64-v8a
@REM set ANDROID_ABI=armeabi-v7a
set ANDROID_ABI=x86_64

set qtSrcPath=c:\qt\qt6-src
@REM if (!(Test-Path $qtSrcPath)) { Read-Host "Qt source not found in $qtSrcPath! Enter to continue."; exit }

set qtBuildPath=c:\qt\qt6-android-build
md %qtBuildPath%
cd %qtBuildPath%

%qtSrcPath%/configure.bat -platform android-clang -prefix c:\qt\qt6-android -android-ndk %ANDROID_NDK_ROOT% -android-sdk %ANDROID_SDK_ROOT% -qt-host-path c:\qt\qt6-windows -android-abis %ANDROID_ABI%
