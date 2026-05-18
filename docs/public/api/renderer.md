# Renderer

The renderer is a small facade for submitting explicit render commands. User
code works with `Renderer`, `Mesh`, `VertexBuffer`, `IndexBuffer`, `Vertex`, and
`Mat4`; bgfx and graphics API details stay inside the renderer backend.

## Headers

```cpp
#include "Renderer.h"
```

## Basic Frame

```cpp
OctalEngine::Renderer renderer;

renderer.beginFrame();
renderer.drawVertices(vertices);
renderer.endFrame();
```

`beginFrame()` opens a command buffer on the calling thread. Draw calls append
commands. `endFrame()` submits the finished buffer to the render thread. These
calls should happen from engine-driven work while `Engine::run()` owns the loop.

## Cube Example

```cpp
#include "Renderer.h"

#include <cstdint>
#include <vector>

std::vector<OctalEngine::Vertex> vertices = {
    {-1, -1, -1, 0, 0, -1, 0, 0},
    { 1, -1, -1, 0, 0, -1, 1, 0},
    { 1,  1, -1, 0, 0, -1, 1, 1},
    {-1,  1, -1, 0, 0, -1, 0, 1},
    {-1, -1,  1, 0, 0,  1, 0, 0},
    { 1, -1,  1, 0, 0,  1, 1, 0},
    { 1,  1,  1, 0, 0,  1, 1, 1},
    {-1,  1,  1, 0, 0,  1, 0, 1},
};

std::vector<std::uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 6, 5, 6, 4, 7,
    0, 4, 5, 5, 1, 0,
    3, 2, 6, 6, 7, 3,
    1, 5, 6, 6, 2, 1,
    0, 3, 7, 7, 4, 0,
};

OctalEngine::RendererInitSettings settings;
settings.headless = false;
settings.nativeWindowHandle = window->nativeHandle();

OctalEngine::Renderer renderer(settings);
OctalEngine::Mesh cube(vertices, indices);

renderer.beginFrame();
renderer.drawMesh(cube, OctalEngine::Mat4::identity());
renderer.endFrame();
```

Headless mode accepts the same commands and discards them on the render thread
without initializing bgfx. For windowed rendering, pass the platform window's
native handle through `RendererInitSettings::nativeWindowHandle`.

## Public Types

- `Renderer`: frame facade and command producer.
- `RendererInitSettings`: startup settings, including `headless`.
- `Mesh`: CPU-side vertex and index storage with an internal resource id.
- `Vertex`: position, normal, and UV data.
- `VertexBuffer` and `IndexBuffer`: small CPU-side buffer wrappers.
- `Mat4`: 4x4 transform matrix.
