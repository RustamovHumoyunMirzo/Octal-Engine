# ResourceManager

The `ResourceManager` provides a simple, thread-safe way to load raw asset files
on background worker threads so the main loop never blocks while waiting for
I/O. It relies on the engine `JobSystem` to run file loads asynchronously.

Key points

- Loads are non-blocking: `LoadAsset()` returns immediately with an
  `AssetHandle`.
- Multiple calls to `LoadAsset()` for the same path reuse an internal entry and
  can register multiple completion callbacks.
- Asset data is stored as a `std::vector<char>` in the returned `Asset` object.
- Filesystem errors are handled and reported via `Asset::status` and
  `Asset::error`.

Header

Include the header in your code:

```cpp
#include "ResourceManager.h"
```

Basic usage

```cpp
using namespace OctalEngine;

// Initialize the resource manager early (starts JobSystem workers if needed)
ResourceManager::Initialize();

// Request load; returns immediately with a handle.
ResourceManager::AssetHandle handle = ResourceManager::LoadAsset("assets/models/mymodel.bin",
    [](ResourceManager::AssetHandle h) {
        auto asset = ResourceManager::GetAsset(h);
        if (asset && asset->status == ResourceManager::Status::Loaded)
        {
            // asset->data contains the raw bytes
        }
        else
        {
            // check asset->status and asset->error
        }
    });

// Later in the main loop you can poll the status without blocking:
auto asset = ResourceManager::GetAsset(handle);
if (asset && asset->status == ResourceManager::Status::Loaded)
{
    // use asset->data
}

// Shutdown the resource manager on exit (waits for pending loads)
ResourceManager::Shutdown();
```

Notes

- Callbacks are invoked from worker threads; if your callback touches
  main-thread-only systems (graphics, UI), dispatch onto the main thread or
  post an event.
- The manager uses `std::filesystem` and `std::ifstream` internally; expect
  platform filesystem semantics.
- This manager provides raw byte loading. Higher-level asset parsers (models,
  textures, materials) should be written on top of the raw `Asset` data and can
  run either on worker threads or on the main thread after load completion.
