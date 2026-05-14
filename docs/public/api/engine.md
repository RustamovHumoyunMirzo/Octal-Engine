# `OctalEngine::Engine`

Header:

```cpp
#include "Engine.h"
```

`Engine` owns the main engine loop. It creates the loop support objects and
repeats update/render frames while the engine is running.

## Constructor

```cpp
Engine();
```

Creates an engine instance. New instances start in the running state.

## Destructor

```cpp
~Engine();
```

Destroys the engine instance. The destructor currently performs no custom
cleanup.

## `run`

```cpp
void run();
```

Starts the main loop.

Current behavior:

- Creates a local `OctalEngine::Time`.
- Creates a local `OctalEngine::GameLoop`.
- Repeats while the engine is running.
- Calls `Time::step()` once per frame.
- Calls `GameLoop::update(dt)` with the returned delta time.
- Calls `GameLoop::render()` after update.

`run()` blocks until the engine is stopped.

Example:

```cpp
#include "Engine.h"

int main()
{
    OctalEngine::Engine engine;
    engine.run();
}
```

## `stop`

```cpp
void stop();
```

Requests the engine loop to stop by setting the engine's internal running flag
to `false`.

The loop observes this flag between frames, so stopping takes effect on the next
loop condition check.

Example:

```cpp
OctalEngine::Engine engine;
engine.stop();
```

Calling `stop()` before `run()` means a later `run()` call will immediately exit
with the current implementation.
