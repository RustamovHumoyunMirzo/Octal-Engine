# Octal Engine

I just wanted to create an easier to use game engine for who want to create games easily. I have been using Roblox Studio, Unity, UE5 for a while, but I wanted something more lightweight and easier to use. So I decided to create my own game engine.

## Timeline

### 26/05/2026
Basic scene rendering with rotating cube, lighting and shadow. (`examples/cube_controller_game.cpp`)

<img src="examples/img/001.png" width="500" height="300" alt="Scene Rendering"/>

## Status

In active development. Not ready for use yet. If you interested in helping out, please reach out to me on Telegram: `@Humoyunbek_R`. Octal Engine currently supports these:
- **Platforms**
    - Windows
- **Graphics APIs**
    - DirectX 11/12
    - Vulkan
    - OpenGL
    - OpenGL ES
    - Metal
    - WebGPU
- **Scene Management**
    - ECS (Entity Component System)
    - Camera
    - Lighting
    - Primitive objects (cube, sphere, plane, etc.)
    - Minimal shadow mapping
- **Input Handling**
    - Keyboard
    - Mouse
    - Gamepad
    - Raw
- **Job System**
    - Thread Pool
    - Task Scheduling
- **Resource Management**
- **Events**

## Navigation

**Building instructions:** [See here](docs/internal/build.md)

**More information about internals:** [See here](docs/internal/info.md)

**What to do right now? (for those who just joined):** [See here](WHAT_TO_DO.md)

**Public Documentation:** [See here](docs/public/README.md)