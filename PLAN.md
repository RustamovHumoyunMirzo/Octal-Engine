# FULL ENGINE PLAN (MVP → SCALABLE ARCHITECTURE)

---

# 0. FINAL TARGET (what you're building toward)

A modular C++ engine that supports:

### Core capabilities

* Cross-platform windowing (SDL2)
* Multiple graphics APIs (bgfx first, Vulkan later optional)
* ECS-based scene system
* Swappable physics (PhysX / Bullet)
* Audio (FMOD)
* Input system
* Resource system
* Headless rendering support
* Future editor layer

---

# 1. CORE ARCHITECTURE (NON-NEGOTIABLE DESIGN)

```text id="arch_core"
Engine (owns everything + loop)
│
├── Platform Layer (SDL2)
│   ├── Window
│   ├── Input
│   ├── OS events
│
├── Renderer Layer (bgfx now)
│   ├── RenderDevice
│   ├── Frame graph (later)
│   ├── GPU resources
│
├── Scene Layer (ECS)
│   ├── EnTT registry
│   ├── Components
│   ├── Systems
│
├── Physics Layer (plugin)
│   ├── PhysX backend
│   ├── Bullet backend
│
├── Audio Layer (FMOD)
│
├── Resource System
│
└── Core Systems
    ├── Time
    ├── Logger
    ├── Job system (later)
```

---

# 2. ENGINE ENTRY POINT (ABSOLUTE TRUTH)

```cpp id="entry"
int main()
{
    Engine engine;
    engine.run();
}
```

Engine owns EVERYTHING.

---

# 3. ENGINE LOOP (FINAL FORM)

```cpp id="loop_final"
void Engine::run()
{
    init();

    while (running)
    {
        float dt = time.step();

        platform.pollEvents(running);

        input.update();

        scene.update(dt);

        physics.step(dt);

        renderer.beginFrame();

        scene.render(renderer);

        renderer.endFrame();

        audio.update();
    }

    shutdown();
}
```

This is your permanent mental model.

---

# 4. PHASED BUILD PLAN (REAL IMPLEMENTATION ROADMAP)

---

# 🟢 PHASE 1 — FOUNDATION (SDL2 + LOOP)

### Goal:

> Get a window + running engine loop

### Build:

#### Platform (SDL2 only)

* SDL window
* input events
* quit handling

#### Engine core

* run loop
* delta time
* clean shutdown

### Result:

✔ Window opens
✔ Loop runs
✔ You can close it

---

# 🟢 PHASE 2 — RENDERING BASE (bgfx)

Using bgfx

### Goal:

> Render something (even just a clear screen)

### Build:

#### Renderer module

* initialize bgfx with SDL window
* frame lifecycle:

  * beginFrame()
  * endFrame()
* clear screen

### Result:

✔ GPU initialized
✔ screen clears every frame
✔ stable FPS loop

---

# 🟢 PHASE 3 — INPUT SYSTEM

### Goal:

> Abstract SDL input into engine input state

### Build:

* keyboard state array
* mouse position + buttons
* event translation layer

### Result:

✔ engine-level input system (not SDL-dependent anymore)

---

# 🟢 PHASE 4 — ECS SCENE SYSTEM

Using EnTT

### Goal:

> Create entities + components

### Build:

* Scene class

* EnTT registry

* Components:

  * Transform
  * Mesh (placeholder)
  * Camera

* Systems:

  * Transform system
  * Render system

### Result:

✔ Entities exist
✔ Scene updates independently

---

# 🟢 PHASE 5 — RENDER PIPELINE

### Goal:

> ECS → renderer integration

### Build:

* RenderSystem collects visible entities
* submits draw calls to renderer
* basic mesh support

### Result:

✔ real scene rendering begins

---

# 🟡 PHASE 6 — PHYSICS ABSTRACTION

Using:

* PhysX
* Bullet Physics

### Goal:

> Swappable physics backend

### Build:

* IPhysicsWorld interface
* PhysicsModule
* RigidBody component
* Sync transforms ECS ↔ physics

### Result:

✔ physics simulation working
✔ backend interchangeable

---

# 🟡 PHASE 7 — AUDIO SYSTEM

Using FMOD

### Build:

* audio device abstraction
* sound sources
* listener (camera)

---

# 🟡 PHASE 8 — RESOURCE SYSTEM

### Build:

* asset manager
* UUID-based asset handles
* caching system
* async loading (later)

Assets:

* textures
* meshes
* shaders
* audio

---

# 🔵 PHASE 9 — JOB SYSTEM (performance layer)

### Build:

* thread pool
* task system
* async loading
* parallel systems:

  * physics
  * rendering prep
  * animation

---

# 🔵 PHASE 10 — HEADLESS + MULTI-RENDER TARGET

### Goal:

> Engine works WITHOUT window

### Build:

* framebuffer renderer mode
* offscreen context
* simulation server mode

---

# 🟣 PHASE 11 — EDITOR FOUNDATION (FUTURE)

### Build:

* reflection system
* component inspector
* scene serializer
* command system (undo/redo)

---

# 5. SDL2 ROLE (VERY IMPORTANT CLARITY)

SDL2 ONLY handles:

✔ window
✔ input
✔ OS events

NOT:

❌ rendering
❌ engine loop
❌ physics
❌ scene

It is just your **platform adapter**, nothing more.

---

# 6. bgfx ROLE

bgfx ONLY handles:

✔ GPU abstraction
✔ rendering backend
✔ frame submission

NOT:

❌ window ownership
❌ ECS
❌ scene logic

---

# 7. FINAL DESIGN RULE (THIS IS THE CORE IDEA)

> Engine owns time
> Platform owns OS
> Renderer owns GPU
> ECS owns world
> Physics owns motion

Nothing else crosses boundaries.

---

# 8. IF YOU FOLLOW THIS EXACT PLAN

You will naturally evolve into:

* Unity-style architecture (data-driven ECS)
* Unreal-style rendering pipeline (modular renderer)
* Editor-ready engine design
* Multi-backend support without rewrite