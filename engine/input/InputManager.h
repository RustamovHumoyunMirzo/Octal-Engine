#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace OctalEngine
{
    enum class InputDeviceType
    {
        Keyboard,
        Mouse,
        Gamepad,
        Touch
    };

    enum class InputValueType
    {
        Digital,
        Axis1D,
        Axis2D
    };

    enum class Key
    {
        Unknown,
        Space,
        Escape,
        Enter,
        W,
        A,
        S,
        D,
        F1
    };

    enum class MouseButton
    {
        Left,
        Middle,
        Right,
        X1,
        X2
    };

    enum class GamepadButton
    {
        South,
        East,
        West,
        North,
        LeftShoulder,
        RightShoulder,
        Start,
        Select
    };

    enum class TouchPhase
    {
        Began,
        Moved,
        Ended,
        Cancelled
    };

    enum class CursorMode
    {
        Free,
        Hidden,
        Locked
    };

    struct InputValue
    {
        float x = 0.0f;
        float y = 0.0f;

        bool down() const
        {
            return x != 0.0f || y != 0.0f;
        }
    };

    struct InputBinding
    {
        InputDeviceType device = InputDeviceType::Keyboard;
        int control = 0;
        float scaleX = 1.0f;
        float scaleY = 0.0f;

        static InputBinding key(Key key, float scaleX = 1.0f, float scaleY = 0.0f);
        static InputBinding mouseButton(MouseButton button);
        static InputBinding gamepadButton(GamepadButton button);
    };

    struct TouchPoint
    {
        std::int64_t id = 0;
        float x = 0.0f;
        float y = 0.0f;
        TouchPhase phase = TouchPhase::Began;
    };

    struct InputContext
    {
        std::string name;
        int priority = 0;
        bool blocksLowerPriority = true;
    };

    struct InputActionEvent
    {
        std::string action;
        InputValue value;
        bool pressed = false;
        bool released = false;
    };

    struct RawButtonEvent
    {
        InputDeviceType device = InputDeviceType::Keyboard;
        int control = 0;
        bool down = false;
    };

    class InputSubscription
    {
    public:
        InputSubscription() = default;
        explicit InputSubscription(std::function<void()> unsubscribe);
        ~InputSubscription();

        InputSubscription(const InputSubscription&) = delete;
        InputSubscription& operator=(const InputSubscription&) = delete;
        InputSubscription(InputSubscription&& other) noexcept;
        InputSubscription& operator=(InputSubscription&& other) noexcept;

        void reset();
        bool active() const;

    private:
        std::function<void()> unsubscribe;
    };

    class InputManager
    {
    public:
        static void defineAction(const std::string& action, InputValueType type = InputValueType::Digital);
        static void removeAction(const std::string& action);

        static void addDefaultBinding(const std::string& context, const std::string& action, InputBinding binding);
        static void addBinding(const std::string& context, const std::string& action, InputBinding binding);
        static void clearBindings(const std::string& context, const std::string& action);
        static void resetToDefaultBindings();
        static bool saveBindings(const std::string& path);
        static bool loadBindings(const std::string& path);

        static void pushContext(InputContext context);
        static void popContext(const std::string& name);
        static void clearContexts();

        static bool isActionDown(const std::string& action);
        static bool wasActionPressedThisFrame(const std::string& action);
        static bool wasActionReleasedThisFrame(const std::string& action);
        static InputValue actionValue(const std::string& action);

        static InputSubscription onAction(std::function<void(const InputActionEvent&)> callback);
        static InputSubscription onRawButton(std::function<void(const RawButtonEvent&)> callback);

        static bool isKeyDown(Key key);
        static bool isMouseButtonDown(MouseButton button);
        static bool isGamepadButtonDown(GamepadButton button);
        static InputValue mouseDelta();
        static std::size_t touchCount();
        static TouchPoint touch(std::size_t index);

        static void setCursorMode(CursorMode mode);
        static CursorMode cursorMode();
        static void setCursorVisible(bool visible);
        static void setCursorLocked(bool locked);
        static bool cursorVisible();
        static bool cursorLocked();

        static void updateFrame(float dt);

        static void feedKey(Key key, bool down);
        static void feedMouseButton(MouseButton button, bool down);
        static void feedMouseDelta(float x, float y);
        static void feedGamepadButton(GamepadButton button, bool down);
        static void feedTouch(TouchPoint touch);

    private:
        InputManager() = default;
    };
}
