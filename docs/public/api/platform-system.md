# `OctalEngine::PlatformSystem`

Header:

```cpp
#include "PlatformSystem.h"
```

`PlatformSystem` is the engine's public platform implementation. It owns
platform subsystem lifetime, pumps platform events, and creates/manages
`OctalEngine::Window` objects.

Link the `OctalEnginePlatform` target to use it:

```cmake
target_link_libraries(MyGame PRIVATE OctalEngine OctalEnginePlatform)
```

## Construction

```cpp
PlatformSystem();
~PlatformSystem() override;
```

The constructor initializes the platform windowing subsystem. If initialization
fails, it throws `std::runtime_error` with a backend error string.

The destructor closes all tracked windows and shuts down the platform windowing
subsystem.

## `createWindow`

```cpp
std::shared_ptr<Window> createWindow(const WindowDescriptor& descriptor = {});
```

Creates a new window and returns an owning `Window` object.

Example:

```cpp
OctalEngine::PlatformSystem platform;

auto window = platform.createWindow({
});
window->setTitle("My Game");
window->setSize(1280, 720);
window->setResizable(true);

OctalEngine::EngineConfig config;
config.mode = OctalEngine::WindowedMode{window.get()};

OctalEngine::Engine engine(platform, config);
```

## Window Management

```cpp
std::shared_ptr<Window> findWindow(WindowId id) const;
void closeWindow(WindowId id);
void closeAllWindows();
std::size_t windowCount() const;
```

Use these functions to find, close, and count currently open windows tracked by
the platform.

## Event Pumping

```cpp
void pumpEvents() override;
bool shouldQuit() const override;
void requestQuit() override;
void clearQuitRequest();
```

`pumpEvents()` handles pending platform events. Application quit events request
platform quit. Window close events close the matching `Window`.

## Last Window Quit Behavior

```cpp
bool quitWhenLastWindowClosed() const;
void setQuitWhenLastWindowClosed(bool enabled);
```

When enabled, the platform requests quit after the last tracked window is
closed. This is enabled by default.
