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

## Public API At A Glance

All public engine classes currently live in the `OctalEngine` namespace.

| Header | Type | Purpose |
| --- | --- | --- |
| `Engine.h` | `OctalEngine::Engine` | Owns the main engine run loop. |
| `EngineTime.h` | `OctalEngine::Time` | Produces the frame delta time used by the loop. |
| `Loop.h` | `OctalEngine::GameLoop` | Provides update and render steps called each frame. |

## Minimal Example

```cpp
#include "Engine.h"

int main()
{
    OctalEngine::Engine engine;
    engine.run();
}
```

`Engine::run()` enters the engine loop and continues until `Engine::stop()` is
called on that same engine instance.

## Current Status

Octal Engine is in active development and is not ready for production use yet.
The documented API reflects the current source code and may change as more
engine systems are added.
