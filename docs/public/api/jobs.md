# Job System

The job system is a small process-wide thread pool for renderer preparation work
and future engine systems.

## Headers

```cpp
#include "JobSystem.h"
```

## Example

```cpp
OctalEngine::JobSystem::initialize();

OctalEngine::JobSystem::dispatch([] {
    // Do one unit of work.
});

OctalEngine::JobSystem::dispatchParallel(128, [](std::size_t index) {
    // Prepare item at index.
});

OctalEngine::JobSystem::wait();
OctalEngine::JobSystem::shutdown();
```

`wait()` blocks until all queued and currently running jobs are complete.
