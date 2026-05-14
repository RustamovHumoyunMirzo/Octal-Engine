# `OctalEngine::Platform`

Header:

```cpp
#include "Platform.h"
```

`Platform` is the small interface used by `Engine` to integrate platform event
pumping without depending on any specific windowing backend.

The core engine does not create or require a `Platform`. Attach one only when
your application needs platform events, windows, or OS integration.

## Destructor

```cpp
virtual ~Platform() = default;
```

Allows platform implementations to be destroyed through the base interface.

## `pumpEvents`

```cpp
virtual void pumpEvents() = 0;
```

Processes pending platform events for the current frame.

`Engine::run()` calls this before update/render when a platform is attached.

## `shouldQuit`

```cpp
virtual bool shouldQuit() const = 0;
```

Returns whether the platform is requesting the engine to stop.

## `requestQuit`

```cpp
virtual void requestQuit() = 0;
```

Requests shutdown from the platform side.
