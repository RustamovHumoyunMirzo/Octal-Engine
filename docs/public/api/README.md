# API Reference

All public classes are in the `OctalEngine` namespace.

## Classes

- [`Engine`](engine.md) - starts and stops the main engine loop.
- [`Time`](time.md) - returns frame delta time values.
- [`GameLoop`](game-loop.md) - update and render operations called by the engine.

## Headers

```cpp
#include "Engine.h"
#include "EngineTime.h"
#include "Loop.h"
```

The project currently exposes these headers through the `OctalEngine` CMake
target's public include directory.
