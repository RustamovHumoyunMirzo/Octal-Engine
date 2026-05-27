# Getting Started

This guide shows how to build Octal Engine and run the current sandbox program.

## Requirements

- C++20 compiler
- CMake 3.20 or newer
- PowerShell if you want to use the helper scripts

## Configure The Build

From the project root:

```powershell
cmake -S . -B build
```

For a specific build type:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

## Build

```powershell
cmake --build build
```

The build creates:

- `OctalEngine` static library
- `OctalEngineJobs` static library
- `OctalEngineRenderer` static library
- `TestSandbox` executable from `test/test.cpp`

## Run The Sandbox

After building, run the generated sandbox executable from your build directory.
The exact path can vary by CMake generator and build configuration.

The current sandbox exercises the scene API. Windowed scene objects with
`MeshGeometry` and `MeshRendererComponent` are rendered internally by `Engine`.

```cpp
#include "Engine.h"
#include "PlatformSystem.h"
#include "Scene.h"

int main()
{
    OctalEngine::PlatformSystem platform;
    auto window = platform.createWindow();

    OctalEngine::EngineConfig config;
    config.mode = OctalEngine::PlatformSystem::windowedModeFor(*window);

    OctalEngine::Engine engine(platform, config);
    auto scene = std::make_unique<OctalEngine::Scene>("Level");

    OctalEngine::Object camera = scene->createObject("Camera");
    camera.addComponent<OctalEngine::TransformComponent>()->position = {0, 0, -6};
    camera.addComponent<OctalEngine::CameraComponent>();
    scene->setPrimaryCamera(camera);

    OctalEngine::Object cube = scene->createObject("Cube");
    cube.addComponent<OctalEngine::TransformComponent>();
    cube.addComponent<OctalEngine::MeshGeometry>();
    cube.addComponent<OctalEngine::MeshRendererComponent>();

    engine.scenes().load(std::move(scene));
    engine.run();
}
```

## Renderer Types

You can specify which rendering API to use when initializing the renderer:

- `RendererType::Auto` - Let bgfx automatically select the best available renderer (default)
- `RendererType::Direct3D11` - DirectX 11
- `RendererType::Direct3D12` - DirectX 12
- `RendererType::OpenGL` - OpenGL
- `RendererType::OpenGLES` - OpenGL ES
- `RendererType::Vulkan` - Vulkan
- `RendererType::Metal` - Metal
- `RendererType::WebGPU` - WebGPU

If an unsupported or unavailable type is requested, the renderer will fall back to auto-detection.

Use `renderer.getRendererType()` to query which API is currently in use:

```cpp
OctalEngine::RendererType activeAPI = renderer.getRendererType();
```

## Include Paths And Linking

For a game project, use the package helper instead of linking engine internals
directly:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyGame)

find_package(OctalEngine REQUIRED)

OctalEngine_AddGame(MyGame
    src/main.cpp
)
```

`OctalEngine_AddGame` creates the executable, applies the required C++ standard,
links the correct Octal Engine targets, and handles runtime backend files when
needed.

## Platform Layer

The engine core does not require a windowing backend. Windowing support is built
as the separate `OctalEnginePlatform` target when `OCTAL_BUILD_PLATFORM` is on:

```powershell
cmake -S . -B build -DOCTAL_BUILD_PLATFORM=ON
```

This option is enabled by default.

To build only the headless core engine target:

```powershell
cmake -S . -B build -DOCTAL_BUILD_PLATFORM=OFF
```

Windowed applications should link both targets:

```cmake
OctalEngine_AddGame(MyGame
    src/main.cpp
)
```

Then create a platform, create a window, and pass the runtime mode into `Engine`:

```cpp
#include "Engine.h"
#include "PlatformSystem.h"

int main()
{
    OctalEngine::PlatformSystem platform;
    auto window = platform.createWindow();

    OctalEngine::EngineConfig config;
    config.mode = OctalEngine::PlatformSystem::windowedModeFor(*window);

    OctalEngine::Engine engine(platform, config);

    engine.run();
}
```
