cmake_minimum_required(VERSION 3.20)
project(OctalEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    add_compile_options(/Zc:__cplusplus /Zc:preprocessor)
    add_compile_definitions($<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>)
endif()

set(OCTALENGINE_VERSION "0.1.0-alpha")

option(OCTAL_BUILD_PLATFORM "Build the platform and windowing layer" ON)
option(OCTAL_BUILD_SANDBOXES "Build Octal Engine sandbox executables" ON)

include(CMakePackageConfigHelpers)

# -----------------------------
# bgfx package
# -----------------------------
list(APPEND CMAKE_PREFIX_PATH
    "${CMAKE_SOURCE_DIR}/third_party/bgfx/cmake"
)

find_package(bgfx CONFIG REQUIRED)

# -----------------------------
# Engine Core
# -----------------------------
file(GLOB_RECURSE ENGINE_CORE_SRC
    engine/core/*.cpp
    engine/input/*.cpp
)

add_library(OctalEngine ${ENGINE_CORE_SRC})
add_library(OctalEngine::OctalEngine ALIAS OctalEngine)

target_include_directories(OctalEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/input>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/platform>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/renderer>
    $<INSTALL_INTERFACE:include/OctalEngine/core>
    $<INSTALL_INTERFACE:include/OctalEngine/input>
    $<INSTALL_INTERFACE:include/OctalEngine/renderer>
)

target_include_directories(OctalEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/third_party/entt>
    $<INSTALL_INTERFACE:include/OctalEngine/third_party/entt>
)

target_include_directories(OctalEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/third_party/tinygltf>
    $<INSTALL_INTERFACE:include/OctalEngine/third_party/tinygltf>
)

# -----------------------------
# Jobs
# -----------------------------
file(GLOB_RECURSE ENGINE_JOBS_SRC
    engine/jobs/*.cpp
)

add_library(OctalEngineJobs ${ENGINE_JOBS_SRC})
add_library(OctalEngine::OctalEngineJobs ALIAS OctalEngineJobs)

target_include_directories(OctalEngineJobs PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/jobs>
    $<INSTALL_INTERFACE:include/OctalEngine/jobs>
)

# -----------------------------
# Assimp
# -----------------------------
add_library(assimp STATIC IMPORTED GLOBAL)

set_target_properties(assimp PROPERTIES
    IMPORTED_LOCATION_DEBUG
        "${CMAKE_SOURCE_DIR}/third_party/assimp/build/lib/Debug/assimp-vc145-mtd.lib"

    IMPORTED_LOCATION_RELEASE
        "${CMAKE_SOURCE_DIR}/third_party/assimp/build/lib/Release/assimp-vc145-mt.lib"

    INTERFACE_INCLUDE_DIRECTORIES
        "${CMAKE_SOURCE_DIR}/third_party/assimp/include"

    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    MAP_IMPORTED_CONFIG_MINSIZEREL Release
)

# -----------------------------
# Renderer
# -----------------------------
file(GLOB_RECURSE ENGINE_RENDERER_SRC
    engine/renderer/*.cpp
)

add_library(OctalEngineRenderer ${ENGINE_RENDERER_SRC})
add_library(OctalEngine::OctalEngineRenderer ALIAS OctalEngineRenderer)

target_include_directories(OctalEngineRenderer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/renderer>
    $<INSTALL_INTERFACE:include/OctalEngine/renderer>
)

target_link_libraries(OctalEngineRenderer
    PRIVATE
        bgfx::bgfx
        bgfx::bx
        bgfx::bimg
        bgfx::bimg_decode
        assimp
)

target_compile_definitions(OctalEngineRenderer PRIVATE
    OCTAL_BGFX_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx/shaders"
    BX_CONFIG_DEBUG=0
)

target_link_libraries(OctalEngine
    PUBLIC OctalEngineRenderer
)

# -----------------------------
# Platform Layer
# -----------------------------
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
        PUBLIC OctalEngine
        PRIVATE SDL2::SDL2
    )

endif()

# -----------------------------
# Sandbox system
# -----------------------------
if(OCTAL_BUILD_SANDBOXES)

    include(cmake/OctalEngine.cmake)

    if(TARGET OctalEnginePlatform)
        OctalEngine_AddGame(TestSandbox test/test.cpp)
    endif()

endif()

# -----------------------------
# Package Config
# -----------------------------
configure_package_config_file(
    cmake/OctalEngineConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/OctalEngineConfig.cmake"
    INSTALL_DESTINATION lib/cmake/OctalEngine
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/OctalEngineConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# -----------------------------
# EXPORT
# -----------------------------
set(EXPORT_TARGETS OctalEngine OctalEngineJobs OctalEngineRenderer)

if(TARGET OctalEnginePlatform)
    list(APPEND EXPORT_TARGETS OctalEnginePlatform)
endif()

export(TARGETS ${EXPORT_TARGETS}
    FILE "${CMAKE_CURRENT_BINARY_DIR}/OctalEngineTargets.cmake"
    NAMESPACE OctalEngine::
)

# -----------------------------
# INSTALL
# -----------------------------
install(TARGETS ${EXPORT_TARGETS}
    EXPORT OctalEngineTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY engine/core/
    DESTINATION include/OctalEngine/core
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY third_party/entt/
    DESTINATION include/OctalEngine/third_party/entt
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY third_party/tinygltf/
    DESTINATION include/OctalEngine/third_party/tinygltf
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

install(DIRECTORY engine/input/
    DESTINATION include/OctalEngine/input
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY engine/jobs/
    DESTINATION include/OctalEngine/jobs
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY engine/renderer/
    DESTINATION include/OctalEngine/renderer
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "backend" EXCLUDE
        PATTERN "internal" EXCLUDE
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
