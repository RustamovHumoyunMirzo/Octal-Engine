# `OctalEngine::Engine`

Header:

```cpp
#include "Engine.h"
```

`Engine` owns the main engine loop. It owns the loop support objects,
optionally pumps a platform supplied at construction, and repeats update/render
frames while the engine is running.

## `EngineConfig` And Modes

```cpp
struct HeadlessMode
{
};

struct WindowedMode
{
    Window* window = nullptr;
    void* nativeWindowHandle = nullptr;
    int width = 1280;
    int height = 720;
};

using RuntimeMode = std::variant<HeadlessMode, WindowedMode>;

struct EngineConfig
{
    RuntimeMode mode = HeadlessMode{};
    bool renderScenes = true;
};
```

`EngineConfig::mode` is a runtime variant. Use `HeadlessMode` for simulations,
tests, tools, or server-like workloads that do not need a window. Use
`WindowedMode` when the engine should run with an existing platform window.
When `nativeWindowHandle` is set, the engine creates an internal renderer for
scene objects.

The engine does not create windows itself and does not require a window to run.

## Constructor

```cpp
Engine();
explicit Engine(const EngineConfig& config);
Engine(Platform& platform, const EngineConfig& config);
```

Creates an engine instance. New instances start in the running state.

The default constructor uses `HeadlessMode`.

Use `Engine(config)` for headless configuration:

```cpp
OctalEngine::EngineConfig config;
config.mode = OctalEngine::HeadlessMode{};

OctalEngine::Engine engine(config);
```

Use `Engine(platform, config)` when the engine should pump platform events:

```cpp
OctalEngine::EngineConfig config;
config.mode = OctalEngine::PlatformSystem::windowedModeFor(*window);

OctalEngine::Engine engine(platform, config);
```

Set `renderScenes` to `false` when a game wants to take full responsibility for
custom rendering.

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

- Pumps the constructor-injected platform if one is present.
- Stops before update/render when the platform requests quit.
- Creates a local `OctalEngine::Time`.
- Repeats while the engine is running.
- Calls `Time::step()` once per frame.
- Emits `Update{dt}` on the engine event bus.
- Calls `GameLoop::update(events, dt)` with the returned delta time.
- Internally renders visible scene objects when scene rendering is enabled.
- Flushes deferred events after the game-loop update.

`run()` blocks until the engine is stopped.

Example:

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

## `events`

```cpp
EventWorld& events();
```

Returns the engine's event world. Use it to subscribe to engine events before
calling `run()`:

```cpp
OctalEngine::Subscription sub = engine.events().engine().subscribe<OctalEngine::Update>(
    [](const OctalEngine::Update& e) {
        printf("%f", e.dt);
    });
```

## `scenes`

```cpp
SceneManager& scenes();
const SceneManager& scenes() const;
```

Returns the scene manager owned by the engine's game loop:

```cpp
engine.scenes().load(std::make_unique<OctalEngine::Scene>("Level01"));
```

## `renderer`

```cpp
Renderer* renderer();
const Renderer* renderer() const;
```

Returns the internal scene renderer after it has been initialized. This can be
`nullptr` before the first rendered frame or when scene rendering is disabled.

## `resizeRenderer`

```cpp
void resizeRenderer(int width, int height);
```

Resizes the internal renderer. Windowed apps should call this from the window
resize event.
