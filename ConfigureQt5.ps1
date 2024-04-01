$env:JDK_ROOT = "C:\Program Files\Android\Android Studio\jbr\bin"
#$env:MINGW_ROOT = "$env:USERPROFILE\Documents\devel\mingw64-win32-msvcrt\bin"
#$env:MINGW_ROOT = "$env:USERPROFILE\Documents\devel\llvm-mingw-20240320-ucrt-x86_64\bin"
$env:MINGW_ROOT = "$env:USERPROFILE\Documents\devel\mingw64-win32-ucrt\bin"
$env:PERL_ROOT = "C:\Strawberry\perl\bin"
$env:PATH = "$env:MINGW_ROOT;$env:PERL_ROOT;$env:JDK_ROOT;$env:PATH"
$sdkPath = "$env:USERPROFILE\AppData\Local\Android\Sdk"
$ndkPath = "$env:USERPROFILE\AppData\Local\Android\Sdk\ndk\25.1.8937393"

$qtSrcPath = "c:\qt\qt5-src"
if (!(Test-Path $qtSrcPath)) { Read-Host "Qt source not found in $qtSrcPath!"; exit }

$qtBuildPath = "c:\qt\build"
if (!(Test-Path $qtBuildPath)) { Write-Host "Will create Qt build path: $qtBuildPath"; mkdir $qtBuildPath }
cd $qtBuildPath

. $qtSrcPath/configure.bat -platform win32-g++ -xplatform android-clang -prefix c:\qt\Qt5 -nomake tests -nomake examples -android-sdk $sdkPath -android-ndk $ndkPath -android-ndk-host windows-x86_64 -no-warnings-are-errors -android-abis arm64-v8a -confirm-license -opensource
