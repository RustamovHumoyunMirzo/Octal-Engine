# Getting Started

This guide shows how to build Octal Engine and run the current sandbox program.

## Requirements

- C++20 compiler
- CMake 3.20 or newer
- PowerShell if you want to use the helper scripts

## Configure The Build

From the project root:

```powershell
cmake -S . -B build
```

For a specific build type:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

## Build

```powershell
cmake --build build
```

The build creates:

- `OctalEngine` static library
- `Sandbox` executable from `test/main.cpp`

## Run The Sandbox

After building, run the generated sandbox executable from your build directory.
The exact path can vary by CMake generator and build configuration.

The current sandbox creates an `OctalEngine::Engine` and calls `run()`:

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

## Include Paths And Linking

The root `CMakeLists.txt` exposes `engine/core` as a public include directory for
the `OctalEngine` target:

```cmake
target_include_directories(OctalEngine PUBLIC engine/core)
```

That means engine users can include public headers directly:

```cpp
#include "Engine.h"
#include "EngineTime.h"
#include "Loop.h"
```

When adding your own executable in CMake, link it with `OctalEngine`:

```cmake
add_executable(MyGame src/main.cpp)
target_link_libraries(MyGame PRIVATE OctalEngine)
```

## Platform Layer

The engine core does not require a windowing backend. Windowing support is built
as the separate `OctalEnginePlatform` target when `OCTAL_BUILD_PLATFORM` is on:

```powershell
cmake -S . -B build -DOCTAL_BUILD_PLATFORM=ON
```

This option is enabled by default.

To build only the headless core engine target:

```powershell
cmake -S . -B build -DOCTAL_BUILD_PLATFORM=OFF
```

Windowed applications should link both targets:

```cmake
target_link_libraries(MyGame PRIVATE OctalEngine OctalEnginePlatform)
```

Then create a platform, create a window, and pass the runtime mode into
`Engine`:

```cpp
#include "Engine.h"
#include "PlatformSystem.h"

int main()
{
    OctalEngine::PlatformSystem platform;
    auto window = platform.createWindow();

    OctalEngine::EngineConfig config;
    config.mode = OctalEngine::WindowedMode{window.get()};

    OctalEngine::Engine engine(platform, config);

    engine.run();
}
```
