cmake_minimum_required(VERSION 3.20)
project(OctalEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OCTALENGINE_VERSION "0.1.0-alpha")
option(OCTAL_BUILD_PLATFORM "Build the platform and windowing layer" ON)
option(OCTAL_BUILD_SANDBOXES "Build Octal Engine sandbox executables" ON)

include(CMakePackageConfigHelpers)

file(GLOB_RECURSE ENGINE_CORE_SRC
    engine/core/*.cpp
)

add_library(OctalEngine ${ENGINE_CORE_SRC})
add_library(OctalEngine::OctalEngine ALIAS OctalEngine)

target_include_directories(OctalEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/core>
    $<INSTALL_INTERFACE:include/OctalEngine/core>
)

if(OCTAL_BUILD_PLATFORM)
    set(SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/cmake" CACHE PATH "Path to SDL2 CMake package")
    find_package(SDL2 CONFIG REQUIRED)

    file(GLOB_RECURSE ENGINE_PLATFORM_SRC
        engine/platform/*.cpp
    )

    add_library(OctalEnginePlatform ${ENGINE_PLATFORM_SRC})
    add_library(OctalEngine::OctalEnginePlatform ALIAS OctalEnginePlatform)

    target_include_directories(OctalEnginePlatform PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/platform>
        $<INSTALL_INTERFACE:include/OctalEngine/platform>
    )

    target_link_libraries(OctalEnginePlatform
        PUBLIC
        OctalEngine
        PRIVATE
        SDL2::SDL2
    )
endif()

set(OCTALENGINE_RUNTIME_DLL "")

if(TARGET OctalEnginePlatform AND WIN32)
    set(OCTALENGINE_RUNTIME_DLL "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/bin/SDL2.dll")
endif()

if(OCTAL_BUILD_SANDBOXES)
    include(cmake/OctalEngine.cmake)

    OctalEngine_AddGame(Sandbox test/main.cpp)

    if(TARGET OctalEnginePlatform)
        OctalEngine_AddGame(WindowedSandbox test/windowed.cpp)
    endif()
endif()

set(OCTALENGINE_SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SDL2/cmake")

configure_package_config_file(
    cmake/OctalEngineConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/OctalEngineConfig.cmake"
    INSTALL_DESTINATION lib/cmake/OctalEngine
)

configure_file(
    cmake/OctalEngine.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/OctalEngine.cmake"
    COPYONLY
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/OctalEngineConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

if(TARGET OctalEnginePlatform)
    export(TARGETS OctalEngine OctalEnginePlatform
        FILE "${CMAKE_CURRENT_BINARY_DIR}/OctalEngineTargets.cmake"
        NAMESPACE OctalEngine::
    )
else()
    export(TARGETS OctalEngine
        FILE "${CMAKE_CURRENT_BINARY_DIR}/OctalEngineTargets.cmake"
        NAMESPACE OctalEngine::
    )
endif()

install(TARGETS OctalEngine
    EXPORT OctalEngineTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

if(TARGET OctalEnginePlatform)
    install(TARGETS OctalEnginePlatform
        EXPORT OctalEngineTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

install(DIRECTORY engine/core/
    DESTINATION include/OctalEngine/core
    FILES_MATCHING PATTERN "*.h"
)

if(TARGET OctalEnginePlatform)
    install(DIRECTORY engine/platform/
        DESTINATION include/OctalEngine/platform
        FILES_MATCHING PATTERN "*.h"
    )
endif()

install(FILES
    cmake/OctalEngine.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/OctalEngineConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/OctalEngineConfigVersion.cmake"
    DESTINATION lib/cmake/OctalEngine
)

install(EXPORT OctalEngineTargets
    NAMESPACE OctalEngine::
    DESTINATION lib/cmake/OctalEngine
)
