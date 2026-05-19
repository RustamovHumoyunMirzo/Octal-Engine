# API Reference

All public classes are in the `OctalEngine` namespace.

## Classes

- [`Engine`](engine.md) - starts and stops the main engine loop.
- [`EngineConfig`](engine.md#engineconfig-and-modes) - runtime mode selection.
- [`Events`](events.md) - typed event buses, subscriptions, and deferred events.
- [`Platform`](platform.md) - optional interface for platform event pumping.
- [`Time`](time.md) - returns frame delta time values.
- [`GameLoop`](game-loop.md) - update and render operations called by the engine.
- [`InputManager`](input.md) - action-based keyboard, mouse, gamepad, and touch input.
- [`JobSystem`](jobs.md) - lightweight thread pool and job dispatch API.
- [`PlatformSystem`](platform-system.md) - platform implementation and window manager.
- [`Renderer`](renderer.md) - render facade, mesh submission, and headless command execution.
- [`Window`](window.md) - window control API.

## Headers

```cpp
#include "Engine.h"
#include "Events.h"
#include "Platform.h"
#include "EngineTime.h"
#include "Loop.h"
#include "InputManager.h"
#include "JobSystem.h"
#include "PlatformSystem.h"
#include "Renderer.h"
#include "Window.h"
```

The project exposes renderer and job headers through their own CMake targets.
`OctalEngine_AddGame` links those targets automatically when they are available.
