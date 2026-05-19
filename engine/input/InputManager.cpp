#include "InputManager.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <unordered_set>

namespace OctalEngine
{
    namespace
    {
        struct BindingEntry
        {
            std::string context;
            std::string action;
            InputBinding binding;
            bool isDefault = false;
        };

        struct ButtonState
        {
            bool down = false;
            bool pressed = false;
            bool released = false;
        };

        struct CallbackState
        {
            std::size_t nextId = 1;
            std::unordered_map<std::size_t, std::function<void(const InputActionEvent&)>> actionCallbacks;
            std::unordered_map<std::size_t, std::function<void(const RawButtonEvent&)>> rawButtonCallbacks;
        };

        struct State
        {
            std::unordered_map<std::string, InputValueType> actions;
            std::vector<BindingEntry> bindings;
            std::vector<BindingEntry> defaultBindings;
            std::vector<InputContext> contexts;
            std::unordered_map<std::uint64_t, ButtonState> buttons;
            InputValue mouseDelta;
            CursorMode cursorMode = CursorMode::Free;
            std::vector<TouchPoint> touches;
            std::shared_ptr<CallbackState> callbacks = std::make_shared<CallbackState>();
        };

        State& state()
        {
            static State input;
            return input;
        }

        std::uint64_t bindingKey(InputDeviceType device, int control)
        {
            return (static_cast<std::uint64_t>(device) << 32) | static_cast<std::uint32_t>(control);
        }

        ButtonState& button(InputDeviceType device, int control)
        {
            return state().buttons[bindingKey(device, control)];
        }

        const ButtonState* findButton(InputDeviceType device, int control)
        {
            auto found = state().buttons.find(bindingKey(device, control));
            return found != state().buttons.end() ? &found->second : nullptr;
        }

        std::vector<std::string> activeContexts()
        {
            auto contexts = state().contexts;
            std::sort(contexts.begin(), contexts.end(), [](const InputContext& a, const InputContext& b) {
                return a.priority > b.priority;
            });

            std::vector<std::string> result;
            for (const InputContext& context : contexts)
            {
                result.push_back(context.name);

                if (context.blocksLowerPriority)
                {
                    break;
                }
            }

            return result;
        }

        bool contextActive(const std::string& context, const std::vector<std::string>& active)
        {
            return std::find(active.begin(), active.end(), context) != active.end();
        }

        std::vector<const BindingEntry*> activeBindingsFor(const std::string& action)
        {
            const std::vector<std::string> active = activeContexts();
            std::vector<const BindingEntry*> result;

            for (const BindingEntry& binding : state().bindings)
            {
                if (binding.action == action && contextActive(binding.context, active))
                {
                    result.push_back(&binding);
                }
            }

            return result;
        }

        void emitRaw(InputDeviceType device, int control, bool down)
        {
            RawButtonEvent event{device, control, down};
            auto callbacks = state().callbacks;

            for (auto& [_, callback] : callbacks->rawButtonCallbacks)
            {
                callback(event);
            }
        }

        void emitActionForBinding(const InputBinding& changedBinding, bool pressed, bool released)
        {
            const std::vector<std::string> active = activeContexts();

            for (const BindingEntry& binding : state().bindings)
            {
                if (binding.binding.device != changedBinding.device ||
                    binding.binding.control != changedBinding.control ||
                    !contextActive(binding.context, active))
                {
                    continue;
                }

                InputActionEvent event;
                event.action = binding.action;
                event.value = InputManager::actionValue(binding.action);
                event.pressed = pressed;
                event.released = released;

                auto callbacks = state().callbacks;
                for (auto& [_, callback] : callbacks->actionCallbacks)
                {
                    callback(event);
                }
            }
        }
    }

    InputBinding InputBinding::key(Key key, float scaleX, float scaleY)
    {
        return {InputDeviceType::Keyboard, static_cast<int>(key), scaleX, scaleY};
    }

    InputBinding InputBinding::mouseButton(MouseButton button)
    {
        return {InputDeviceType::Mouse, static_cast<int>(button), 1.0f, 0.0f};
    }

    InputBinding InputBinding::gamepadButton(GamepadButton button)
    {
        return {InputDeviceType::Gamepad, static_cast<int>(button), 1.0f, 0.0f};
    }

    InputSubscription::InputSubscription(std::function<void()> unsubscribe)
        : unsubscribe(std::move(unsubscribe))
    {
    }

    InputSubscription::~InputSubscription()
    {
        reset();
    }

    InputSubscription::InputSubscription(InputSubscription&& other) noexcept
        : unsubscribe(std::move(other.unsubscribe))
    {
        other.unsubscribe = nullptr;
    }

    InputSubscription& InputSubscription::operator=(InputSubscription&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            unsubscribe = std::move(other.unsubscribe);
            other.unsubscribe = nullptr;
        }

        return *this;
    }

    void InputSubscription::reset()
    {
        if (unsubscribe != nullptr)
        {
            unsubscribe();
            unsubscribe = nullptr;
        }
    }

    bool InputSubscription::active() const
    {
        return unsubscribe != nullptr;
    }

    void InputManager::defineAction(const std::string& action, InputValueType type)
    {
        state().actions[action] = type;
    }

    void InputManager::removeAction(const std::string& action)
    {
        state().actions.erase(action);
        state().bindings.erase(
            std::remove_if(state().bindings.begin(), state().bindings.end(), [&action](const BindingEntry& binding) {
                return binding.action == action;
            }),
            state().bindings.end());
    }

    void InputManager::addDefaultBinding(const std::string& context, const std::string& action, InputBinding binding)
    {
        defineAction(action);
        state().bindings.push_back(BindingEntry{context, action, binding, true});
        state().defaultBindings.push_back(BindingEntry{context, action, binding, true});
    }

    void InputManager::addBinding(const std::string& context, const std::string& action, InputBinding binding)
    {
        defineAction(action);
        state().bindings.push_back(BindingEntry{context, action, binding, false});
    }

    void InputManager::clearBindings(const std::string& context, const std::string& action)
    {
        state().bindings.erase(
            std::remove_if(state().bindings.begin(), state().bindings.end(), [&](const BindingEntry& binding) {
                return binding.context == context && binding.action == action;
            }),
            state().bindings.end());
    }

    void InputManager::resetToDefaultBindings()
    {
        state().bindings = state().defaultBindings;
    }

    bool InputManager::saveBindings(const std::string& path)
    {
        std::ofstream file(path);
        if (!file)
        {
            return false;
        }

        for (const BindingEntry& binding : state().bindings)
        {
            file << binding.context << ' ' << binding.action << ' '
                 << static_cast<int>(binding.binding.device) << ' '
                 << binding.binding.control << ' '
                 << binding.binding.scaleX << ' '
                 << binding.binding.scaleY << '\n';
        }

        return true;
    }

    bool InputManager::loadBindings(const std::string& path)
    {
        std::ifstream file(path);
        if (!file)
        {
            return false;
        }

        state().bindings.clear();
        std::string context;
        std::string action;
        int device = 0;
        int control = 0;
        float scaleX = 0.0f;
        float scaleY = 0.0f;

        while (file >> context >> action >> device >> control >> scaleX >> scaleY)
        {
            state().bindings.push_back(BindingEntry{
                context,
                action,
                InputBinding{static_cast<InputDeviceType>(device), control, scaleX, scaleY},
                false});
        }

        return true;
    }

    void InputManager::pushContext(InputContext context)
    {
        popContext(context.name);
        state().contexts.push_back(std::move(context));
    }

    void InputManager::popContext(const std::string& name)
    {
        state().contexts.erase(
            std::remove_if(state().contexts.begin(), state().contexts.end(), [&name](const InputContext& context) {
                return context.name == name;
            }),
            state().contexts.end());
    }

    void InputManager::clearContexts()
    {
        state().contexts.clear();
    }

    bool InputManager::isActionDown(const std::string& action)
    {
        for (const BindingEntry* binding : activeBindingsFor(action))
        {
            const ButtonState* state = findButton(binding->binding.device, binding->binding.control);
            if (state != nullptr && state->down)
            {
                return true;
            }
        }

        return false;
    }

    bool InputManager::wasActionPressedThisFrame(const std::string& action)
    {
        for (const BindingEntry* binding : activeBindingsFor(action))
        {
            const ButtonState* state = findButton(binding->binding.device, binding->binding.control);
            if (state != nullptr && state->pressed)
            {
                return true;
            }
        }

        return false;
    }

    bool InputManager::wasActionReleasedThisFrame(const std::string& action)
    {
        for (const BindingEntry* binding : activeBindingsFor(action))
        {
            const ButtonState* state = findButton(binding->binding.device, binding->binding.control);
            if (state != nullptr && state->released)
            {
                return true;
            }
        }

        return false;
    }

    InputValue InputManager::actionValue(const std::string& action)
    {
        InputValue value;

        for (const BindingEntry* binding : activeBindingsFor(action))
        {
            const ButtonState* state = findButton(binding->binding.device, binding->binding.control);
            if (state != nullptr && state->down)
            {
                value.x += binding->binding.scaleX;
                value.y += binding->binding.scaleY;
            }
        }

        return value;
    }

    InputSubscription InputManager::onAction(std::function<void(const InputActionEvent&)> callback)
    {
        auto callbacks = state().callbacks;
        const std::size_t id = callbacks->nextId++;
        callbacks->actionCallbacks[id] = std::move(callback);

        return InputSubscription([weakCallbacks = std::weak_ptr<CallbackState>(callbacks), id]() {
            if (auto locked = weakCallbacks.lock())
            {
                locked->actionCallbacks.erase(id);
            }
        });
    }

    InputSubscription InputManager::onRawButton(std::function<void(const RawButtonEvent&)> callback)
    {
        auto callbacks = state().callbacks;
        const std::size_t id = callbacks->nextId++;
        callbacks->rawButtonCallbacks[id] = std::move(callback);

        return InputSubscription([weakCallbacks = std::weak_ptr<CallbackState>(callbacks), id]() {
            if (auto locked = weakCallbacks.lock())
            {
                locked->rawButtonCallbacks.erase(id);
            }
        });
    }

    bool InputManager::isKeyDown(Key key)
    {
        const ButtonState* state = findButton(InputDeviceType::Keyboard, static_cast<int>(key));
        return state != nullptr && state->down;
    }

    bool InputManager::isMouseButtonDown(MouseButton button)
    {
        const ButtonState* state = findButton(InputDeviceType::Mouse, static_cast<int>(button));
        return state != nullptr && state->down;
    }

    bool InputManager::isGamepadButtonDown(GamepadButton gamepadButton)
    {
        const ButtonState* state = findButton(InputDeviceType::Gamepad, static_cast<int>(gamepadButton));
        return state != nullptr && state->down;
    }

    InputValue InputManager::mouseDelta()
    {
        return state().mouseDelta;
    }

    std::size_t InputManager::touchCount()
    {
        return state().touches.size();
    }

    TouchPoint InputManager::touch(std::size_t index)
    {
        if (index >= state().touches.size())
        {
            return {};
        }

        return state().touches[index];
    }

    void InputManager::setCursorMode(CursorMode mode)
    {
        state().cursorMode = mode;
    }

    CursorMode InputManager::cursorMode()
    {
        return state().cursorMode;
    }

    void InputManager::setCursorVisible(bool visible)
    {
        state().cursorMode = visible ? CursorMode::Free : CursorMode::Hidden;
    }

    void InputManager::setCursorLocked(bool locked)
    {
        state().cursorMode = locked ? CursorMode::Locked : CursorMode::Free;
    }

    bool InputManager::cursorVisible()
    {
        return state().cursorMode != CursorMode::Hidden;
    }

    bool InputManager::cursorLocked()
    {
        return state().cursorMode == CursorMode::Locked;
    }

    void InputManager::updateFrame(float)
    {
        for (auto& [_, button] : state().buttons)
        {
            button.pressed = false;
            button.released = false;
        }

        state().mouseDelta = {};
        state().touches.erase(
            std::remove_if(state().touches.begin(), state().touches.end(), [](const TouchPoint& touch) {
                return touch.phase == TouchPhase::Ended || touch.phase == TouchPhase::Cancelled;
            }),
            state().touches.end());
    }

    void InputManager::feedKey(Key key, bool down)
    {
        ButtonState& current = button(InputDeviceType::Keyboard, static_cast<int>(key));
        const bool changed = current.down != down;
        current.pressed = changed && down;
        current.released = changed && !down;
        current.down = down;
        emitRaw(InputDeviceType::Keyboard, static_cast<int>(key), down);
        emitActionForBinding(InputBinding::key(key), current.pressed, current.released);
    }

    void InputManager::feedMouseButton(MouseButton mouseButton, bool down)
    {
        ButtonState& current = button(InputDeviceType::Mouse, static_cast<int>(mouseButton));
        const bool changed = current.down != down;
        current.pressed = changed && down;
        current.released = changed && !down;
        current.down = down;
        emitRaw(InputDeviceType::Mouse, static_cast<int>(mouseButton), down);
        emitActionForBinding(InputBinding::mouseButton(mouseButton), current.pressed, current.released);
    }

    void InputManager::feedMouseDelta(float x, float y)
    {
        state().mouseDelta.x += x;
        state().mouseDelta.y += y;
    }

    void InputManager::feedGamepadButton(GamepadButton gamepadButton, bool down)
    {
        ButtonState& current = button(InputDeviceType::Gamepad, static_cast<int>(gamepadButton));
        const bool changed = current.down != down;
        current.pressed = changed && down;
        current.released = changed && !down;
        current.down = down;
        emitRaw(InputDeviceType::Gamepad, static_cast<int>(gamepadButton), down);
        emitActionForBinding(InputBinding::gamepadButton(gamepadButton), current.pressed, current.released);
    }

    void InputManager::feedTouch(TouchPoint touch)
    {
        auto found = std::find_if(state().touches.begin(), state().touches.end(), [&](const TouchPoint& existing) {
            return existing.id == touch.id;
        });

        if (found == state().touches.end())
        {
            state().touches.push_back(touch);
        }
        else
        {
            *found = touch;
        }
    }
}
