# `OctalEngine::GameLoop`

Header:

```cpp
#include "Loop.h"
```

`GameLoop` contains the per-frame update and render functions called by
`OctalEngine::Engine`.

## `update`

```cpp
void update(float dt);
```

Runs one update step.

Parameters:

- `dt` - delta time for the current frame, in seconds.

The current implementation is a placeholder and does not modify engine state.

Example:

```cpp
#include "Loop.h"

OctalEngine::GameLoop loop;
loop.update(1.0f / 60.0f);
```

## `render`

```cpp
void render();
```

Runs one render step.

The current implementation writes this message to standard output:

```text
OctalEngine frame
```

Example:

```cpp
#include "Loop.h"

OctalEngine::GameLoop loop;
loop.render();
```
