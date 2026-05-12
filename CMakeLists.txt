cmake_minimum_required(VERSION 3.20)
project(OctalEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE ENGINE_SRC
    engine/core/*.cpp
)

add_library(OctalEngine ${ENGINE_SRC})

target_include_directories(OctalEngine PUBLIC engine/core)

add_executable(Sandbox test/main.cpp)

target_link_libraries(Sandbox PRIVATE OctalEngine)