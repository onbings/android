# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("demolvgl")
    set(VCPKG_TARGET_TRIPLET "x64-android-32")
    set(VCPKG_ROOT "C:/pro/github/vcpkg")
    include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        demolvgl

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        main.cpp

        AndroidOut.cpp
        Renderer.cpp
        Shader.cpp
        TextureAsset.cpp
        Utility.cpp
		AndroidLvglDrv.cpp
		AndroidLvglDrv.h
		)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Searches for a package provided by the game activity dependency

find_package(game-activity REQUIRED CONFIG)

#set(CMAKE_FIND_ROOT_PATH C:/Android/repo/32)
set(CMAKE_FIND_DEBUG_MODE TRUE)
find_package(bofstd REQUIRED)
find_package(bof2d REQUIRED)
find_package(SDL2 REQUIRED)
find_package(lvgl REQUIRED)

message("==BOFSTD_FOUND======>" ${BOFSTD_FOUND})
message("==BOFSTD_INCLUDE_DIRS======>" ${BOFSTD_INCLUDE_DIRS})
message("==BOFSTD_LIBRARIES======>" ${BOFSTD_LIBRARIES})

message("==BOF2D_FOUND======>" ${BOF2D_FOUND})
message("==BOF2D_INCLUDE_DIRS======>" ${BOF2D_INCLUDE_DIRS})
message("==BOF2D_LIBRARIES======>" ${BOF2D_LIBRARIES})

message("==LVGL_FOUND======>" ${LVGL_FOUND})
message("==LVGL_INCLUDE_DIRS======>" ${LVGL_INCLUDE_DIRS})
message("==LVGL_LIBRARIES======>" ${LVGL_LIBRARIES})

message("==SDL2_FOUND======>" ${SDL2_FOUND})
message("==SDL2_INCLUDE_DIRS======>" ${SDL2_INCLUDE_DIRS})
message("==SDL2_LIBRARIES======>" ${SDL2_LIBRARIES})


target_compile_definitions(demolvgl
        PUBLIC

        PRIVATE
        -DBOF2D_STATIC_DEFINE=1	#in lower case to define the SYMBOL BOFSTD_EXPORT (upper case)in bof2d_export.h
        )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        demolvgl

        android

        # The game activity
        game-activity::game-activity

        # EGL, required for configuring the display context
        EGL

        # GL ES 3, used for the sample renderer
        GLESv3

        # for AImageDecoder, to load images from resources
        jnigraphics

        ONBINGS::bofstd
        ONBINGS::bof2d
        ONBINGS::lvgl
        SDL2::SDL2main
        SDL2::SDL2 #SDL2-static
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})