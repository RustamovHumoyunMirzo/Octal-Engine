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
object.addComponent<OctalEngine::MeshGeometry>();
object.addComponent<OctalEngine::MeshRendererComponent>();
```

`Object` is a small public handle. It does not expose the internal ECS registry
or entity type. Users can derive from `Object` for their own object-facing
wrappers, and can derive component data from `OctalEngine::Component`.

## Default Components

- `TransformComponent` - local `position`, `scale`, and quaternion `rotation`.
- `CameraComponent` - `fov`, `nearPlane`, `farPlane`, and `isOrthographic`.
- `MeshGeometry` - defines the `primitive` type (Cube, Sphere, Plane, Capsule, Cylinder, Custom).
- `MeshRendererComponent` - controls rendering with `visible`, `castShadows`, `receiveShadows`, `sortingOrder`, and `renderLayer`.
- `LightComponent` - `intensity`, `color`, light type, shadow controls, and `castShadows`.

Components that need a transform are skipped when the object has no transform.
For the default set this applies to camera, mesh renderer, and light. The call
returns `nullptr` when a component is skipped.

To render a mesh, an object must have both `MeshGeometry` and `MeshRendererComponent`
components. If either is missing, the object will be skipped during rendering.

`MeshRendererComponent::castShadows` controls whether the object casts shadows into
the shadow map. `receiveShadows` controls whether shadows are rendered on the
object's surface. Objects are sorted for rendering by `renderLayer` first, then
by `sortingOrder` within the same layer.

### Shadow System

The engine implements a dynamic shadow system with realistic shadow physics:

- **Cast Shadows** (`castShadows = true`): Objects with this flag render their shadow geometry onto shadow receivers.
- **Receive Shadows** (`receiveShadows = true`): Objects with this flag display shadows cast by other objects.
- Both flags work independently - an object can cast shadows without receiving them, or vice versa.
- Directional lights can enable shadow casting with `LightComponent::castShadows = true`.
- Shadow intensity and bias are controlled through `shadowStrength` and `shadowBias`.

### Render Order

Objects are rendered in this order:
1. Sorted by `renderLayer` (ascending) - allows grouping objects into background/midground/foreground
2. Within each layer, sorted by `sortingOrder` (ascending) - fine-grained control within a layer

Example: Layer 0 (all objects) → Layer 1 (sorted by sortingOrder) → Layer 2 (sorted by sortingOrder)

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
