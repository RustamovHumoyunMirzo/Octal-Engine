# Scene Management

Header:

```cpp
#include "Scene.h"
```

Scenes own an isolated ECS world. Loading a new scene creates a separate world;
objects and components do not carry across scene swaps.

## SceneManager

```cpp
OctalEngine::SceneManager& scenes = engine.scenes();
scenes.load(std::make_unique<OctalEngine::Scene>("Level01"));
```

`SceneManager` lives in `GameLoop` and is updated on the main loop thread.
`load()` queues the next scene and the swap is applied during
`SceneManager::update()`. `unload()` clears the active scene on the next update.

## Objects

```cpp
OctalEngine::Object object = scene.createObject("Cube");
object.addComponent<OctalEngine::TransformComponent>();
object.addComponent<OctalEngine::MeshRendererComponent>();
```

`Object` is a small public handle. It does not expose the internal ECS registry
or entity type. Users can derive from `Object` for their own object-facing
wrappers, and can derive component data from `OctalEngine::Component`.

## Default Components

- `TransformComponent` - local `position`, `scale`, and quaternion `rotation`.
- `CameraComponent` - `fov`, `nearPlane`, `farPlane`, and `isOrthographic`.
- `MeshRendererComponent` - primitive type, `visible`, and `castShadows`.
- `LightComponent` - `intensity`, `color`, light type, shadow controls, and
  `castShadows`.

Components that need a transform are skipped when the object has no transform.
For the default set this applies to camera, mesh renderer, and light. The call
returns `nullptr` when a component is skipped.

`MeshRendererComponent::castShadows` controls shadow-map render inclusion. It
does not mean the object performs lighting by itself.

Directional lights read their direction from the transform on the same object.
Use `object.forward()` or `scene.forwardOf(object)` to read the world-space
direction.

Visible mesh renderers are rendered by the engine's internal scene renderer
when `EngineConfig::renderScenes` is enabled. `PrimitiveType::Custom` is
reserved for future custom mesh support and is skipped for now.

## Primary Camera

```cpp
scene.setPrimaryCamera(cameraObject);
OctalEngine::Object camera = scene.primaryCamera();
```

The primary camera is stored on the scene. `setPrimaryCamera()` only accepts an
object that currently has a `CameraComponent`. The internal scene renderer uses
this camera when rendering the active scene.

## Parent And Child Objects

```cpp
child.setParent(parent);
OctalEngine::TransformComponent world = child.worldTransform();
```

Objects can be parented without requiring transforms. When transforms exist,
children inherit their parent transform relatively. Cycles are ignored.

## Scene Events

Scene events are emitted through `EventWorld::scene()`:

- `SceneLoaded`
- `SceneUnloaded`
- `SceneActivated`
- `SceneDeactivated`
- `SceneUpdated`
- `ObjectCreated`
- `ObjectDestroyed`
