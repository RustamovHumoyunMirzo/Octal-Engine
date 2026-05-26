# `OctalEngine::GameLoop`

Header:

```cpp
#include "Loop.h"
```

`GameLoop` contains the per-frame update work called by `OctalEngine::Engine`.
It owns the main-thread `SceneManager`.

## `scenes`

```cpp
SceneManager& scenes();
const SceneManager& scenes() const;
```

Returns the scene manager owned by the loop.

## `update`

```cpp
void update(EventWorld& events, float dt);
void update(float dt);
```

Runs one update step. The engine-facing overload attaches the scene manager to
the engine event world, advances input frame state, and updates the active
scene.

Parameters:

- `dt` - delta time for the current frame, in seconds.

Example:

```cpp
#include "Loop.h"

OctalEngine::GameLoop loop;
OctalEngine::EventWorld events;
loop.update(events, 1.0f / 60.0f);
```
