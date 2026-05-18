# Octal Engine Public Documentation

Octal Engine is an early-stage C++20 game engine library. The public API is
currently small and centered around starting the engine loop, stepping time, and
submitting explicit renderer commands.

These docs are written for people using the engine from their own C++ code.
Internal design notes live in [`../internal`](../internal/).

## Start Here

- [Getting Started](getting-started.md) - build the engine and run the sandbox.
- [Engine Loop Concepts](engine-loop.md) - understand how `Engine`, `Time`, and
  `GameLoop` work together.
- [API Reference](api/README.md) - public classes and member functions.
- [Events](api/events.md) - typed event buses and frame-safe deferred events.

## Game CMake

Game projects can use the package helper without knowing engine internals:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyGame)

find_package(OctalEngine REQUIRED)

OctalEngine_AddGame(MyGame
    src/main.cpp
)
```

## Public API At A Glance

All public engine classes currently live in the `OctalEngine` namespace.

| Header | Type | Purpose |
| --- | --- | --- |
| `Engine.h` | `OctalEngine::Engine`, `OctalEngine::EngineConfig` | Owns the main engine run loop and runtime mode config. |
| `Events.h` | `OctalEngine::EventWorld`, `OctalEngine::EventBus`, `OctalEngine::Subscription` | Typed event buses and listener lifetime. |
| `Platform.h` | `OctalEngine::Platform` | Optional event-pumping interface for integrations. |
| `EngineTime.h` | `OctalEngine::Time` | Produces the frame delta time used by the loop. |
| `Loop.h` | `OctalEngine::GameLoop` | Provides update and render steps called each frame. |
| `JobSystem.h` | `OctalEngine::JobSystem` | Dispatches work to a small thread pool. |
| `Renderer.h` | `OctalEngine::Renderer`, `OctalEngine::Mesh` | Submits mesh and vertex draw commands to the render thread. |
| `PlatformSystem.h` | `OctalEngine::PlatformSystem` | Platform and window manager. |
| `Window.h` | `OctalEngine::Window` | Runtime window control API. |

## Minimal Example

```cpp
#include "Engine.h"

int main()
{
    OctalEngine::EngineConfig config;
    config.mode = OctalEngine::HeadlessMode{};

    OctalEngine::Engine engine(config);

    engine.run();
}
```

`Engine::run()` enters the engine loop and continues until `Engine::stop()` is
called on that same engine instance.

## Windowed Example

The engine can run without a window. To use platform windows, create a
`PlatformSystem`, create one or more windows, and pass the platform and runtime
mode into the engine constructor:

```cpp
#include "Engine.h"
#include "PlatformSystem.h"

int main()
{
    OctalEngine::PlatformSystem platform;

    OctalEngine::WindowDescriptor windowDescriptor;
    windowDescriptor.title = "My Game";
    windowDescriptor.width = 1280;
    windowDescriptor.height = 720;

    auto window = platform.createWindow(windowDescriptor);

    OctalEngine::EngineConfig config;
    config.mode = OctalEngine::WindowedMode{window.get()};

    OctalEngine::Engine engine(platform, config);

engine.run();
}
```

## Cube Render Command Example

The renderer currently executes user-issued render commands. Submit those
commands from work driven by `engine.run()`; user code should not own the engine
loop.

```cpp
#include "Renderer.h"

#include <cstdint>
#include <vector>

int main()
{
    std::vector<OctalEngine::Vertex> vertices = {
        {-1, -1, -1, 0, 0, -1, 0, 0},
        { 1, -1, -1, 0, 0, -1, 1, 0},
        { 1,  1, -1, 0, 0, -1, 1, 1},
        {-1,  1, -1, 0, 0, -1, 0, 1},
        {-1, -1,  1, 0, 0,  1, 0, 0},
        { 1, -1,  1, 0, 0,  1, 1, 0},
        { 1,  1,  1, 0, 0,  1, 1, 1},
        {-1,  1,  1, 0, 0,  1, 0, 1},
    };

    std::vector<std::uint16_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 6, 5, 6, 4, 7,
        0, 4, 5, 5, 1, 0,
        3, 2, 6, 6, 7, 3,
        1, 5, 6, 6, 2, 1,
        0, 3, 7, 7, 4, 0,
    };

    OctalEngine::RendererInitSettings settings;
    settings.headless = false;
    settings.nativeWindowHandle = window->nativeHandle();

    OctalEngine::Renderer renderer(settings);
    OctalEngine::Mesh cube(vertices, indices);
    OctalEngine::Engine engine(config);

    auto renderCube = engine.events().engine().subscribe<OctalEngine::Update>(
        [&renderer, &cube](const OctalEngine::Update&) {
            renderer.beginFrame();
            renderer.drawMesh(cube, OctalEngine::Mat4::identity());
            renderer.endFrame();
        });

    engine.run();
}
```

## Current Status

Octal Engine is in active development and is not ready for production use yet.
The documented API reflects the current source code and may change as more
engine systems are added.
