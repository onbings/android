cls
set pwd=%cd%
set ANDROID_NDK_HOME=C:\Android\Sdk\ndk\25.1.8937393
mkdir c:\Android\repo\bofstd
mkdir c:\Android\bld\bofstd\Debug
mkdir c:\Android\bld\bofstd\Release
cd c:\Android\bld\bofstd\Debug
cmake -DVCPKG_TARGET_TRIPLET=x64-android-30 -DANDROID_PLATFORM=android-30 -DANDROID_ABI=x86_64  -DCMAKE_TOOLCHAIN_FILE=C:/pro/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=C:/Android/Sdk/ndk/25.1.8937393/build/cmake/android.toolchain.cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DVCPKG_OVERLAY_PORTS=C:/pro/github/onbings-vcpkg-registry/ports -DCMAKE_DEBUG_POSTFIX=_d C:/pro/github/bofstd
cmake --build .
REM cmake --install .
cd %pwd%