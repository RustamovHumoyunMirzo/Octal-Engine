# `OctalEngine::Time`

Header:

```cpp
#include "EngineTime.h"
```

`Time` provides frame delta time values for the engine loop.

## `step`

```cpp
float step();
```

Returns the delta time for the next frame.

Current behavior:

```cpp
return 1.0f / 60.0f;
```

This means the current engine loop behaves as if each frame takes one sixtieth
of a second. The value is fixed and is not currently measured from wall-clock
time.

Example:

```cpp
#include "EngineTime.h"

OctalEngine::Time time;
float dt = time.step();
```

Use the returned value when updating simulation or gameplay state:

```cpp
position += velocity * dt;
```
