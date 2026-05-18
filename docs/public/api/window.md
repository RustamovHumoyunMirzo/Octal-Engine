# `OctalEngine::Window`

Header:

```cpp
#include "Window.h"
```

`Window` controls one OS window. Windows are created through
`PlatformSystem::createWindow()`.

## Supporting Types

```cpp
using WindowId = std::uint32_t;

struct WindowSize
{
    int width = 1280;
    int height = 720;
};

struct WindowPosition
{
    int x = 0;
    int y = 0;
};

struct WindowDescriptor
{
    std::string title = "Octal Engine";
    int width = 1280;
    int height = 720;
    int x = -1;
    int y = -1;
    bool centered = true;
    bool resizable = true;
    bool visible = true;
    bool fullscreen = false;
};
```

## Lifetime

```cpp
~Window();
Window(const Window&) = delete;
Window& operator=(const Window&) = delete;
Window(Window&&) noexcept;
Window& operator=(Window&&) noexcept;
```

`Window` owns its native window. Destroying or closing the `Window` destroys the
native window. Windows cannot be copied.

## Identity And State

```cpp
WindowId id() const;
bool isOpen() const;
void close();
void* nativeHandle() const;
```

`nativeHandle()` returns the underlying backend window handle as `void*`. Use it
only when integrating with lower-level rendering or platform APIs.

## Title

```cpp
std::string title() const;
void setTitle(const std::string& title);
```

Gets or changes the window title.

## Size And Position

```cpp
WindowSize size() const;
void setSize(int width, int height);

WindowPosition position() const;
void setPosition(int x, int y);
void center();
```

Controls the window's size and screen position.

## Visibility And Window Mode

```cpp
void show();
void hide();
void maximize();
void minimize();
void restore();

bool setFullscreen(bool enabled);
void setResizable(bool enabled);
```

These functions expose common runtime window controls. `setFullscreen()` returns
`false` if the native window is closed or the backend rejects the mode change.

## Per-Window Events

```cpp
WindowEvents& events();

auto resized = window->events().onResize(
    [](const OctalEngine::WindowResized& event) {
        // event.width, event.height
    });

auto minimized = window->events().onMinimize(
    [](const OctalEngine::WindowMinimized&) {
    });
```

Window events are attached to the specific `Window` returned by
`PlatformSystem::createWindow()`. There is no global window event bus.

Available callbacks:

- `onResize(WindowResized)`
- `onMove(WindowMoved)`
- `onMinimize(WindowMinimized)`
- `onMaximize(WindowMaximized)`
- `onRestore(WindowRestored)`
- `onShow(WindowShown)`
- `onHide(WindowHidden)`
- `onFocusGained(WindowFocusGained)`
- `onFocusLost(WindowFocusLost)`
- `onCloseRequested(WindowCloseRequested)`

Keep the returned `WindowSubscription` alive for as long as the callback should
remain active.
