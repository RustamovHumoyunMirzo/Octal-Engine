#include "PlatformSystem.h"
#include "InputManager.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cstdint>
#include <stdexcept>

namespace OctalEngine
{
    namespace
    {
        Key translateKey(SDL_Keycode key)
        {
            switch (key)
            {
            case SDLK_SPACE:
                return Key::Space;
            case SDLK_ESCAPE:
                return Key::Escape;
            case SDLK_RETURN:
                return Key::Enter;
            case SDLK_w:
                return Key::W;
            case SDLK_a:
                return Key::A;
            case SDLK_s:
                return Key::S;
            case SDLK_d:
                return Key::D;
            case SDLK_F1:
                return Key::F1;
            default:
                return Key::Unknown;
            }
        }

        MouseButton translateMouseButton(std::uint8_t button)
        {
            switch (button)
            {
            case SDL_BUTTON_LEFT:
                return MouseButton::Left;
            case SDL_BUTTON_MIDDLE:
                return MouseButton::Middle;
            case SDL_BUTTON_RIGHT:
                return MouseButton::Right;
            case SDL_BUTTON_X1:
                return MouseButton::X1;
            case SDL_BUTTON_X2:
                return MouseButton::X2;
            default:
                return MouseButton::Left;
            }
        }

        GamepadButton translateGamepadButton(std::uint8_t button)
        {
            switch (button)
            {
            case SDL_CONTROLLER_BUTTON_A:
                return GamepadButton::South;
            case SDL_CONTROLLER_BUTTON_B:
                return GamepadButton::East;
            case SDL_CONTROLLER_BUTTON_X:
                return GamepadButton::West;
            case SDL_CONTROLLER_BUTTON_Y:
                return GamepadButton::North;
            case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                return GamepadButton::LeftShoulder;
            case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                return GamepadButton::RightShoulder;
            case SDL_CONTROLLER_BUTTON_START:
                return GamepadButton::Start;
            case SDL_CONTROLLER_BUTTON_BACK:
                return GamepadButton::Select;
            default:
                return GamepadButton::South;
            }
        }

        TouchPhase translateTouchPhase(std::uint32_t eventType)
        {
            switch (eventType)
            {
            case SDL_FINGERDOWN:
                return TouchPhase::Began;
            case SDL_FINGERMOTION:
                return TouchPhase::Moved;
            case SDL_FINGERUP:
                return TouchPhase::Ended;
            default:
                return TouchPhase::Cancelled;
            }
        }
    }

    PlatformSystem::PlatformSystem()
    {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0)
        {
            throw std::runtime_error(SDL_GetError());
        }

        for (int i = 0; i < SDL_NumJoysticks(); ++i)
        {
            openGameController(i);
        }
    }

    PlatformSystem::~PlatformSystem()
    {
        closeGameControllers();
        closeAllWindows();
        SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS);
    }

    std::shared_ptr<Window> PlatformSystem::createWindow(const WindowDescriptor& descriptor)
    {
        Uint32 flags = 0;

        if (descriptor.resizable)
        {
            flags |= SDL_WINDOW_RESIZABLE;
        }

        if (!descriptor.visible)
        {
            flags |= SDL_WINDOW_HIDDEN;
        }

        if (descriptor.fullscreen)
        {
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }

        const int x = descriptor.centered ? SDL_WINDOWPOS_CENTERED : descriptor.x;
        const int y = descriptor.centered ? SDL_WINDOWPOS_CENTERED : descriptor.y;

        SDL_Window* nativeWindow = SDL_CreateWindow(
            descriptor.title.c_str(),
            x,
            y,
            descriptor.width,
            descriptor.height,
            flags);

        if (nativeWindow == nullptr)
        {
            throw std::runtime_error(SDL_GetError());
        }

        auto window = std::shared_ptr<Window>(new Window(nativeWindow));
        windows.push_back(window);
        return window;
    }

    std::shared_ptr<Window> PlatformSystem::findWindow(WindowId id) const
    {
        for (const auto& weakWindow : windows)
        {
            auto window = weakWindow.lock();

            if (window != nullptr && window->id() == id && window->isOpen())
            {
                return window;
            }
        }

        return nullptr;
    }

    void PlatformSystem::closeWindow(WindowId id)
    {
        auto window = findWindow(id);

        if (window != nullptr)
        {
            window->close();
        }

        pruneClosedWindows();
    }

    void PlatformSystem::closeAllWindows()
    {
        for (const auto& weakWindow : windows)
        {
            auto window = weakWindow.lock();

            if (window != nullptr)
            {
                window->close();
            }
        }

        windows.clear();
    }

    std::size_t PlatformSystem::windowCount() const
    {
        return std::count_if(windows.begin(), windows.end(), [](const std::weak_ptr<Window>& weakWindow) {
            auto window = weakWindow.lock();
            return window != nullptr && window->isOpen();
        });
    }

    void PlatformSystem::pumpEvents()
    {
        SDL_ShowCursor(InputManager::cursorMode() == CursorMode::Hidden ? SDL_DISABLE : SDL_ENABLE);
        SDL_SetRelativeMouseMode(InputManager::cursorMode() == CursorMode::Locked ? SDL_TRUE : SDL_FALSE);

        SDL_Event event;

        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
            {
                requestQuit();
                continue;
            }

            if (event.type == SDL_CONTROLLERDEVICEADDED)
            {
                openGameController(event.cdevice.which);
                continue;
            }

            if (event.type == SDL_CONTROLLERDEVICEREMOVED)
            {
                closeGameController(event.cdevice.which);
                continue;
            }

            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                const Key key = translateKey(event.key.keysym.sym);

                if (key != Key::Unknown && event.key.repeat == 0)
                {
                    InputManager::feedKey(key, event.type == SDL_KEYDOWN);
                }

                continue;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
            {
                InputManager::feedMouseButton(translateMouseButton(event.button.button), event.type == SDL_MOUSEBUTTONDOWN);
                continue;
            }

            if (event.type == SDL_MOUSEMOTION)
            {
                InputManager::feedMouseDelta(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
                continue;
            }

            if (event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP)
            {
                InputManager::feedGamepadButton(
                    translateGamepadButton(event.cbutton.button),
                    event.type == SDL_CONTROLLERBUTTONDOWN);
                continue;
            }

            if (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERMOTION || event.type == SDL_FINGERUP)
            {
                InputManager::feedTouch({
                    static_cast<std::int64_t>(event.tfinger.fingerId),
                    event.tfinger.x,
                    event.tfinger.y,
                    translateTouchPhase(event.type)});
                continue;
            }

            if (event.type == SDL_WINDOWEVENT)
            {
                auto window = findWindow(event.window.windowID);

                if (window != nullptr)
                {
                    window->handlePlatformEvent(event.window.event, event.window.data1, event.window.data2);
                }

                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    closeWindow(event.window.windowID);
                }
            }
        }

        pruneClosedWindows();

        if (quitOnLastWindowClosed && windows.empty())
        {
            requestQuit();
        }
    }

    bool PlatformSystem::shouldQuit() const
    {
        return quitRequested;
    }

    void PlatformSystem::requestQuit()
    {
        quitRequested = true;
    }

    void PlatformSystem::clearQuitRequest()
    {
        quitRequested = false;
    }

    bool PlatformSystem::quitWhenLastWindowClosed() const
    {
        return quitOnLastWindowClosed;
    }

    void PlatformSystem::setQuitWhenLastWindowClosed(bool enabled)
    {
        quitOnLastWindowClosed = enabled;
    }

    void PlatformSystem::pruneClosedWindows()
    {
        windows.erase(
            std::remove_if(windows.begin(), windows.end(), [](const std::weak_ptr<Window>& weakWindow) {
                auto window = weakWindow.lock();
                return window == nullptr || !window->isOpen();
            }),
            windows.end());
    }

    void PlatformSystem::openGameController(int deviceIndex)
    {
        if (SDL_IsGameController(deviceIndex) == SDL_FALSE)
        {
            return;
        }

        SDL_GameController* controller = SDL_GameControllerOpen(deviceIndex);
        if (controller != nullptr)
        {
            gameControllers.push_back(controller);
        }
    }

    void PlatformSystem::closeGameController(int instanceId)
    {
        gameControllers.erase(
            std::remove_if(gameControllers.begin(), gameControllers.end(), [instanceId](void* controller) {
                SDL_GameController* gameController = static_cast<SDL_GameController*>(controller);
                SDL_Joystick* joystick = SDL_GameControllerGetJoystick(gameController);

                if (joystick == nullptr || SDL_JoystickInstanceID(joystick) != instanceId)
                {
                    return false;
                }

                SDL_GameControllerClose(gameController);
                return true;
            }),
            gameControllers.end());
    }

    void PlatformSystem::closeGameControllers()
    {
        for (void* controller : gameControllers)
        {
            SDL_GameControllerClose(static_cast<SDL_GameController*>(controller));
        }

        gameControllers.clear();
    }
}
