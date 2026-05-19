# Input Manager

The input manager is action-first. Gameplay code should ask for actions such as
`Move`, `Jump`, `Shoot`, or `Interact`; raw keys and buttons are kept as binding
data and low-level escape hatches.

Threading rule: input is single-threaded today. Platform code feeds input during
event pumping, gameplay reads input during engine update, and `GameLoop::update`
advances the input frame state.

## Public Gameplay API

```cpp
#include "InputManager.h"

OctalEngine::InputManager::defineAction("Move", OctalEngine::InputValueType::Axis2D);
OctalEngine::InputManager::defineAction("Shoot");

OctalEngine::InputManager::addDefaultBinding(
    "Gameplay",
    "Move",
    OctalEngine::InputBinding::key(OctalEngine::Key::A, -1.0f, 0.0f));

OctalEngine::InputManager::pushContext({"Gameplay", 0, true});

bool shooting = OctalEngine::InputManager::isActionDown("Shoot");
bool shotStarted = OctalEngine::InputManager::wasActionPressedThisFrame("Shoot");
bool shotEnded = OctalEngine::InputManager::wasActionReleasedThisFrame("Shoot");
OctalEngine::InputValue movement = OctalEngine::InputManager::actionValue("Move");
```

Core polling functions:

- `isActionDown(action)`
- `wasActionPressedThisFrame(action)`
- `wasActionReleasedThisFrame(action)`
- `actionValue(action)`

## Contexts

Contexts let higher-level modes override lower-level modes:

```cpp
OctalEngine::InputManager::pushContext({"Gameplay", 0, true});
OctalEngine::InputManager::pushContext({"Menu", 100, true});
OctalEngine::InputManager::popContext("Menu");
```

Higher priority contexts are evaluated first. A blocking context prevents lower
priority contexts from contributing actions.

## Rebinding

```cpp
OctalEngine::InputManager::clearBindings("Gameplay", "Shoot");
OctalEngine::InputManager::addBinding(
    "Gameplay",
    "Shoot",
    OctalEngine::InputBinding::key(OctalEngine::Key::Space));

OctalEngine::InputManager::saveBindings("bindings.input");
OctalEngine::InputManager::loadBindings("bindings.input");
OctalEngine::InputManager::resetToDefaultBindings();
```

Actions can have multiple bindings across keyboard, mouse, gamepad, and future
devices.

## Events

Polling is the main gameplay path. Events are available for UI and systems:

```cpp
auto sub = OctalEngine::InputManager::onAction(
    [](const OctalEngine::InputActionEvent& event) {
        if (event.action == "Shoot" && event.pressed) {
        }
    });
```

Raw button events are also available:

```cpp
auto raw = OctalEngine::InputManager::onRawButton(
    [](const OctalEngine::RawButtonEvent& event) {
    });
```

Keep the returned `InputSubscription` alive while the callback should remain
active.

## Raw Escape Hatch

```cpp
bool w = OctalEngine::InputManager::isKeyDown(OctalEngine::Key::W);
bool left = OctalEngine::InputManager::isMouseButtonDown(OctalEngine::MouseButton::Left);
bool south = OctalEngine::InputManager::isGamepadButtonDown(OctalEngine::GamepadButton::South);
OctalEngine::InputValue delta = OctalEngine::InputManager::mouseDelta();
```

Gameplay should prefer actions. Raw access exists for tooling, debug panels,
text input plumbing, and platform-specific integration.

## Cursor And Touch

```cpp
OctalEngine::InputManager::setCursorMode(OctalEngine::CursorMode::Locked);
OctalEngine::InputManager::setCursorMode(OctalEngine::CursorMode::Free);
OctalEngine::InputManager::setCursorMode(OctalEngine::CursorMode::Hidden);

std::size_t count = OctalEngine::InputManager::touchCount();
OctalEngine::TouchPoint point = OctalEngine::InputManager::touch(0);
```

Touch points carry an id, normalized position, and phase.

## System-Level API

Platform backends feed input through system calls:

```cpp
OctalEngine::InputManager::feedKey(OctalEngine::Key::W, true);
OctalEngine::InputManager::feedMouseButton(OctalEngine::MouseButton::Left, false);
OctalEngine::InputManager::feedMouseDelta(dx, dy);
OctalEngine::InputManager::feedGamepadButton(OctalEngine::GamepadButton::South, true);
OctalEngine::InputManager::feedTouch(touchPoint);
```

The engine advances frame state from `GameLoop::update(float dt)`:

```cpp
OctalEngine::InputManager::updateFrame(dt);
```

## Lifecycle

1. Platform pumps SDL events.
2. Platform translates raw events into input feed calls.
3. Gameplay reads actions during the engine update.
4. `GameLoop::update()` clears frame transitions and mouse delta.

This keeps `pressed this frame` and `released this frame` deterministic even
with variable frame rates.

## Minimal Core Subset

The most important API surface for gameplay:

```cpp
defineAction(action, type);
addDefaultBinding(context, action, binding);
pushContext(context);
isActionDown(action);
wasActionPressedThisFrame(action);
wasActionReleasedThisFrame(action);
actionValue(action);
onAction(callback);
```
