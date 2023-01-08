#echo off
REM c:\pro\android\cmp_prj C:\pro\github\bofstd x86_64 32 x64 Debug
REM c:\pro\android\cmp_prj C:\pro\github\bof2d x86_64 32 x64 Debug
REM c:\pro\android\cmp_prj C:\pro\github\lvgl x86_64 32 x64 Debug

REM c:\pro\android\cmp_prj C:\pro\github\bofstd x86_64 32 x64 Release
REM c:\pro\android\cmp_prj C:\pro\github\bofstd x86 30 x86 Debug
cls
set CPP_FULL_PATH=%~f1
set CPP_DIR=%~dp1
set CPP_PRJ=%~nx1

echo BUILD '%CPP_PRJ%' in '%CPP_DIR%'
set pwd=%cd%
set ANDROID_NDK_HOME=C:\Android\Sdk\ndk\25.1.8937393
set VCPKG_ROOT=C:/pro/vcpkg
set CPP_ANDROID_PORT_FILE=C:/pro/github/onbings-vcpkg-registry/ports
set CPP_ANDROID_ROOT=C:\Android
REM x86_64
set CPP_ANDROID_ABI=%2
REM 30
set CPP_ANDROID_API=%3

set CPP_ANDROID_BLD=%CPP_ANDROID_ROOT%\bld\%CPP_ANDROID_API%\%CPP_ANDROID_ABI%
set CPP_ANDROID_REPO=%CPP_ANDROID_ROOT%\repo\%CPP_ANDROID_API%

REM x64
set CPP_ANDROID_TRIPLET=%4-android-%CPP_ANDROID_API%
mkdir %CPP_ANDROID_REPO%\%CPP_PRJ%
REM Debug
set CPP_TYPE=%5
set CPP_LIB_EXT=
if %CPP_TYPE%==Debug set CPP_LIB_EXT=_d


mkdir %CPP_ANDROID_BLD%\%CPP_PRJ%\%CPP_TYPE%
cd %CPP_ANDROID_BLD%\%CPP_PRJ%\%CPP_TYPE%

cmake -Wno-dev -DVCPKG_TARGET_TRIPLET=%CPP_ANDROID_TRIPLET% -DANDROID_PLATFORM=android-%CPP_ANDROID_API% -DANDROID_ABI=%CPP_ANDROID_ABI% -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=%ANDROID_NDK_HOME%/build/cmake/android.toolchain.cmake -GNinja -DCMAKE_BUILD_TYPE=%CPP_TYPE% -DVCPKG_OVERLAY_PORTS=%CPP_ANDROID_PORT_FILE% -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=lib/%CPP_ANDROID_ABI%/%CPP_TYPE% -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=lib/%CPP_ANDROID_ABI%/%CPP_TYPE% -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=bin/%CPP_ANDROID_ABI%/%CPP_TYPE% -DCMAKE_DEBUG_POSTFIX=%CPP_LIB_EXT% %CPP_FULL_PATH%
cmake --build . --config %CPP_TYPE%
cmake --install . --prefix %CPP_ANDROID_REPO%/%CPP_PRJ%
%VCPKG_ROOT%/vcpkg install %CPP_PRJ%:%CPP_ANDROID_TRIPLET% --overlay-ports=%CPP_ANDROID_PORT_FILE% 

%VCPKG_ROOT%/vcpkg list
cd %pwd%
