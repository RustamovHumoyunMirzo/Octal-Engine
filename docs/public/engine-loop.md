# Engine Loop Concepts

Octal Engine currently exposes a simple fixed-step style loop:

```cpp
OctalEngine::Time time;
OctalEngine::GameLoop loop;

while (running)
{
    float dt = time.step();

    loop.update(dt);
    loop.render();
}
```

This is the same flow used internally by `OctalEngine::Engine::run()`.

## Frame Delta Time

`Time::step()` returns the delta time passed into `GameLoop::update(float dt)`.
At the moment, this value is fixed at `1.0f / 60.0f`.

Because timing is currently fixed, users should treat `dt` as a frame-duration
hint rather than a measured wall-clock value.

## Update Then Render

Each frame calls:

1. `GameLoop::update(dt)`
2. `GameLoop::render()`

Use `update(dt)` for simulation and gameplay state changes. Use `render()` for
drawing or presenting the current frame.

## Stopping The Loop

`Engine::stop()` changes the engine's internal running flag to `false`.
`Engine::run()` checks that flag at the start of each loop iteration.

The current public API does not yet expose callbacks, scenes, input dispatch, or
an event system. Code that needs to stop the engine must have access to the
running `Engine` instance.
