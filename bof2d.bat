cls
set pwd=%cd%
set ANDROID_NDK_HOME=C:\Android\Sdk\ndk\25.1.8937393
mkdir c:\Android\repo\bof2d
mkdir c:\Android\bld\bof2d\Debug
mkdir c:\Android\bld\bof2d\Release

cd c:\Android\bld\bof2d\Debug
cmake -Wno-dev -DVCPKG_TARGET_TRIPLET=x64-android-30 -DANDROID_PLATFORM=android-30 -DANDROID_ABI=x86_64 -DCMAKE_TOOLCHAIN_FILE=C:/pro/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=C:/Android/Sdk/ndk/25.1.8937393/build/cmake/android.toolchain.cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DVCPKG_OVERLAY_PORTS=C:/pro/github/onbings-vcpkg-registry/ports -DCMAKE_DEBUG_POSTFIX=_d C:/pro/github/bof2d
cmake --build . --config Debug
cmake --install . --prefix C:/Android/repo/bof2d

cd c:\Android\bld\bof2d\Release
cmake -Wno-dev -DVCPKG_TARGET_TRIPLET=x64-android-30 -DANDROID_PLATFORM=android-30 -DANDROID_ABI=x86_64 -DCMAKE_TOOLCHAIN_FILE=C:/pro/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=C:/Android/Sdk/ndk/25.1.8937393/build/cmake/android.toolchain.cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DVCPKG_OVERLAY_PORTS=C:/pro/github/onbings-vcpkg-registry/ports C:/pro/github/bof2d
cmake --build . --config Release
cmake --install . --prefix C:/Android/repo/bof2d

cd %pwd%