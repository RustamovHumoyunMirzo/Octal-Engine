# Events

Header:

```cpp
#include "Events.h"
```

Octal Engine provides a typed event system built around `EventWorld`,
`EventBus`, and RAII `Subscription` handles.

## EventWorld

```cpp
class EventWorld
{
public:
    EventBus& engine();
    EventBus& scene();
    EventBus& physics();
    void flush();
};
```

`EventWorld` groups engine-level, scene-level, and physics-level event buses.
Use `flush()` to run deferred events queued on all buses.

## Subscribe

```cpp
Subscription sub = events.engine().subscribe<Update>(
    [](const Update& e) {
        printf("%f", e.dt);
    });
```

`Subscription` is move-only and owns the listener lifetime. Destroying it
automatically unsubscribes:

```cpp
{
    Subscription sub = events.engine().subscribe<Update>([](const Update& e) {
        printf("%f", e.dt);
    });
} // listener removed here
```

## Once

```cpp
events.engine().once<Update>([](const auto& e) {
    printf("%f", e.dt);
});
```

`once()` registers a listener that removes itself after the first matching
event. You do not need to store a `Subscription`.

## Immediate Emit

```cpp
events.engine().emit<Update>({dt});
```

Immediate events run listeners before `emit()` returns.

## Deferred Emit

```cpp
events.engine().emitDeferred<Update>({dt});
events.flush();
```

Deferred events are queued until `flush()` runs. This is useful for frame-safe
work where listeners should run after the current operation finishes.

## Built-In Events

```cpp
struct Update
{
    float dt = 0.0f;
};
```

`Engine::run()` emits `Update` on the engine bus each frame before the current
placeholder `GameLoop::update(dt)` call. The engine flushes deferred events once
per frame after the current placeholder render step.
