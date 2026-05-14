cmake_minimum_required(VERSION 3.20)
project(OctalEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(OCTAL_BUILD_PLATFORM "Build the platform and windowing layer" ON)

file(GLOB_RECURSE ENGINE_CORE_SRC
    engine/core/*.cpp
)

add_library(OctalEngine ${ENGINE_CORE_SRC})

target_include_directories(OctalEngine PUBLIC
    engine/core
)

if(OCTAL_BUILD_PLATFORM)
    set(SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/cmake" CACHE PATH "Path to SDL2 CMake package")
    find_package(SDL2 CONFIG REQUIRED)

    file(GLOB_RECURSE ENGINE_PLATFORM_SRC
        engine/platform/*.cpp
    )

    add_library(OctalEnginePlatform ${ENGINE_PLATFORM_SRC})

    target_include_directories(OctalEnginePlatform PUBLIC
        engine/platform
    )

    target_link_libraries(OctalEnginePlatform
        PUBLIC
        OctalEngine
        PRIVATE
        SDL2::SDL2
    )
endif()

add_executable(Sandbox test/main.cpp)

target_link_libraries(Sandbox PRIVATE OctalEngine)

if(TARGET OctalEnginePlatform)
    add_executable(WindowedSandbox test/windowed.cpp)

    target_link_libraries(WindowedSandbox PRIVATE
        OctalEngine
        OctalEnginePlatform
    )

    if(WIN32)
        add_custom_command(TARGET WindowedSandbox POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/bin/SDL2.dll"
                "$<TARGET_FILE_DIR:WindowedSandbox>"
        )
    endif()
endif()
