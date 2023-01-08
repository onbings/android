REM Add set(VCPKG_CMAKE_SYSTEM_VERSION 30) To C:\pro\vcpkg\triplets\community\x64-android.cmake to specify adroid api version (default 21)

REM
REM 1. Check the presence of required environment variables
REM
set ANDROID_NDK_HOME=C:/Android/Sdk/ndk/21.4.7075529
set VCPKG_ROOT=C:/pro/vcpkg

REM
REM 2. Set the path to the toolchains
REM
set vcpkg_toolchain_file=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
set android_toolchain_file=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
set VCPKG_CMAKE_SYSTEM_VERSION=30
REM
REM 3. Select a pair "Android abi" / "vcpkg triplet"
REM Uncomment one of the four possibilities below
REM

REM android_abi=armeabi-v7a
REM vcpkg_target_triplet=arm-android

REM android_abi=x86
REM vcpkg_target_triplet=x86-android

REM android_abi=arm64-v8a
REM vcpkg_target_triplet=arm64-android

set android_abi=x86_64
set vcpkg_target_triplet=x64-android

REM
REM 4. Install the library via vcpkg
REM
%VCPKG_ROOT%/vcpkg install bof2d:%vcpkg_target_triplet% --overlay-ports=C:/pro/github/onbings-vcpkg-registry/ports/

