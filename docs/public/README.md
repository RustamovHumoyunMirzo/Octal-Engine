# Octal Engine Public Documentation

Octal Engine is an early-stage C++20 game engine library. The public API is
currently small and centered around starting the engine loop, stepping time, and
running update/render work.

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

## Current Status

Octal Engine is in active development and is not ready for production use yet.
The documented API reflects the current source code and may change as more
engine systems are added.
