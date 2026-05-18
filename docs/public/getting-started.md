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

The current sandbox creates a window, creates an engine, and submits a cube mesh
through `OctalEngine::Renderer` from inside `engine.run()`.

```cpp
#include "Engine.h"
#include "Renderer.h"

int main()
{
    OctalEngine::RendererInitSettings settings;
    settings.headless = false;
    settings.nativeWindowHandle = window->nativeHandle();

    OctalEngine::Renderer renderer(settings);
    OctalEngine::Mesh cube(vertices, indices);

    OctalEngine::Engine engine(config);
    auto renderCube = engine.events().engine().subscribe<OctalEngine::Update>(
        [&renderer, &cube](const OctalEngine::Update&) {
            renderer.beginFrame();
            renderer.drawMesh(cube);
            renderer.endFrame();
        });

    engine.run();
}
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
    config.mode = OctalEngine::WindowedMode{window.get()};

    OctalEngine::Engine engine(platform, config);

    engine.run();
}
```
