# API Reference

All public classes are in the `OctalEngine` namespace.

## Classes

- [`Engine`](engine.md) - starts and stops the main engine loop.
- [`EngineConfig`](engine.md#engineconfig-and-modes) - runtime mode selection.
- [`Events`](events.md) - typed event buses, subscriptions, and deferred events.
- [`Platform`](platform.md) - optional interface for platform event pumping.
- [`Time`](time.md) - returns frame delta time values.
- [`GameLoop`](game-loop.md) - update and render operations called by the engine.
- [`PlatformSystem`](platform-system.md) - platform implementation and window manager.
- [`Window`](window.md) - window control API.

## Headers

```cpp
#include "Engine.h"
#include "Events.h"
#include "Platform.h"
#include "EngineTime.h"
#include "Loop.h"
#include "PlatformSystem.h"
#include "Window.h"
```

The project currently exposes these headers through the `OctalEngine` CMake
target's public include directory.
