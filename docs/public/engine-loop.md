# Engine Loop Concepts

Octal Engine currently exposes a simple fixed-step style loop:

```cpp
OctalEngine::Time time;
OctalEngine::GameLoop loop;

while (running)
{
    pumpPlatformIfAvailable();

    float dt = time.step();

    events.engine().emit<Update>({dt});
    loop.update(dt);
    loop.render();
    events.flush();
}
```

This is the same flow used internally by `OctalEngine::Engine::run()`.

## Runtime Modes

`EngineConfig` stores the runtime mode as a variant:

```cpp
OctalEngine::EngineConfig config{
};
config.mode = OctalEngine::HeadlessMode{};
```

Use `HeadlessMode` when the engine should update without a window. Use
`WindowedMode` when a platform window exists:

```cpp
OctalEngine::EngineConfig config;
config.mode = OctalEngine::WindowedMode{window.get()};

OctalEngine::Engine engine(platform, config);
```

The engine pumps platform events internally when constructed with a platform. If
the platform requests quit, the engine stops before the next update.

Headless mode does not use presentation or window state. The update loop still
runs normally.

## Frame Delta Time

`Time::step()` returns the delta time passed into `GameLoop::update(float dt)`.
At the moment, this value is fixed at `1.0f / 60.0f`.

Because timing is currently fixed, users should treat `dt` as a frame-duration
hint rather than a measured wall-clock value.

## Update Then Render

Each frame calls:

1. `GameLoop::update(dt)`
2. `GameLoop::render()`

Use `update(dt)` for simulation and gameplay state changes. Rendering is exposed
through `OctalEngine::Renderer`, which records user-issued draw commands between
`beginFrame()` and `endFrame()` and submits them to a render thread. User code
should submit those commands from engine-driven work, such as an update event,
while `Engine::run()` remains the owner of the frame loop.

## Engine Update Event

The engine emits `Update{dt}` on the engine event bus each frame:

```cpp
OctalEngine::Subscription sub = engine.events().engine().subscribe<OctalEngine::Update>(
    [](const OctalEngine::Update& e) {
        printf("%f", e.dt);
    });
```

Deferred events queued during a frame are flushed after the current placeholder
render step.

## Stopping The Loop

`Engine::stop()` changes the engine's internal running flag to `false`.
`Engine::run()` checks that flag at the start of each loop iteration.

The current public API does not yet expose callbacks, scenes, input dispatch, or
an event system. Code that needs to stop the engine must have access to the
running `Engine` instance.
