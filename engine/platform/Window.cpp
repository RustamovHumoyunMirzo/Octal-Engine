#include "Window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

namespace OctalEngine
{
    struct Window::Impl
    {
        explicit Impl(SDL_Window* window)
            : window(window), id(window != nullptr ? SDL_GetWindowID(window) : 0)
        {
        }

        SDL_Window* window = nullptr;
        WindowId id = 0;
        WindowEvents events;
    };

    WindowSubscription::WindowSubscription(std::function<void()> unsubscribe)
        : unsubscribe(std::move(unsubscribe))
    {
    }

    WindowSubscription::~WindowSubscription()
    {
        reset();
    }

    WindowSubscription::WindowSubscription(WindowSubscription&& other) noexcept
        : unsubscribe(std::move(other.unsubscribe))
    {
        other.unsubscribe = nullptr;
    }

    WindowSubscription& WindowSubscription::operator=(WindowSubscription&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            unsubscribe = std::move(other.unsubscribe);
            other.unsubscribe = nullptr;
        }

        return *this;
    }

    void WindowSubscription::reset()
    {
        if (unsubscribe != nullptr)
        {
            unsubscribe();
            unsubscribe = nullptr;
        }
    }

    bool WindowSubscription::active() const
    {
        return unsubscribe != nullptr;
    }

    WindowEvents::WindowEvents()
        : alive(std::make_shared<bool>(true))
    {
    }

    WindowEvents::~WindowEvents()
    {
        if (alive != nullptr)
        {
            *alive = false;
        }
    }

    WindowSubscription WindowEvents::onResize(std::function<void(const WindowResized&)> callback)
    {
        return subscribe(resizedHandlers, std::move(callback));
    }

    WindowSubscription WindowEvents::onMove(std::function<void(const WindowMoved&)> callback)
    {
        return subscribe(movedHandlers, std::move(callback));
    }

    WindowSubscription WindowEvents::onMinimize(std::function<void(const WindowMinimized&)> callback)
    {
        return subscribe(minimizedHandlers, std::move(callback));
    }

    WindowSubscription WindowEvents::onMaximize(std::function<void(const WindowMaximized&)> callback)
    {
        return subscribe(maximizedHandlers, std::move(callback));
    }

    WindowSubscription WindowEvents::onRestore(std::function<void(const WindowRestored&)> callback)
    {
        return subscribe(restoredHandlers, std::move(callback));
    }

    WindowSubscription WindowEvents::onShow(std::function<void(const WindowShown&)> callback)
    {
        return subscribe(shownHandlers, std::move(callback));
    }

    WindowSubscription WindowEvents::onHide(std::function<void(const WindowHidden&)> callback)
    {
        return subscribe(hiddenHandlers, std::move(callback));
    }

    WindowSubscription WindowEvents::onFocusGained(std::function<void(const WindowFocusGained&)> callback)
    {
        return subscribe(focusGainedHandlers, std::move(callback));
    }

    WindowSubscription WindowEvents::onFocusLost(std::function<void(const WindowFocusLost&)> callback)
    {
        return subscribe(focusLostHandlers, std::move(callback));
    }

    WindowSubscription WindowEvents::onCloseRequested(std::function<void(const WindowCloseRequested&)> callback)
    {
        return subscribe(closeRequestedHandlers, std::move(callback));
    }

    Window::Window(void* nativeWindow)
        : impl(std::make_unique<Impl>(static_cast<SDL_Window*>(nativeWindow)))
    {
    }

    Window::~Window()
    {
        close();
    }

    Window::Window(Window&&) noexcept = default;

    Window& Window::operator=(Window&& other) noexcept
    {
        if (this != &other)
        {
            close();
            impl = std::move(other.impl);
        }

        return *this;
    }

    WindowId Window::id() const
    {
        return impl != nullptr ? impl->id : 0;
    }

    bool Window::isOpen() const
    {
        return impl != nullptr && impl->window != nullptr;
    }

    void Window::close()
    {
        if (isOpen())
        {
            SDL_DestroyWindow(impl->window);
            impl->window = nullptr;
        }
    }

    std::string Window::title() const
    {
        if (!isOpen())
        {
            return {};
        }

        const char* value = SDL_GetWindowTitle(impl->window);
        return value != nullptr ? value : "";
    }

    void Window::setTitle(const std::string& title)
    {
        if (isOpen())
        {
            SDL_SetWindowTitle(impl->window, title.c_str());
        }
    }

    WindowSize Window::size() const
    {
        WindowSize result{};

        if (isOpen())
        {
            SDL_GetWindowSize(impl->window, &result.width, &result.height);
        }

        return result;
    }

    void Window::setSize(int width, int height)
    {
        if (isOpen())
        {
            SDL_SetWindowSize(impl->window, width, height);
        }
    }

    WindowPosition Window::position() const
    {
        WindowPosition result{};

        if (isOpen())
        {
            SDL_GetWindowPosition(impl->window, &result.x, &result.y);
        }

        return result;
    }

    void Window::setPosition(int x, int y)
    {
        if (isOpen())
        {
            SDL_SetWindowPosition(impl->window, x, y);
        }
    }

    void Window::center()
    {
        if (isOpen())
        {
            SDL_SetWindowPosition(impl->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
    }

    void Window::show()
    {
        if (isOpen())
        {
            SDL_ShowWindow(impl->window);
        }
    }

    void Window::hide()
    {
        if (isOpen())
        {
            SDL_HideWindow(impl->window);
        }
    }

    void Window::maximize()
    {
        if (isOpen())
        {
            SDL_MaximizeWindow(impl->window);
        }
    }

    void Window::minimize()
    {
        if (isOpen())
        {
            SDL_MinimizeWindow(impl->window);
        }
    }

    void Window::restore()
    {
        if (isOpen())
        {
            SDL_RestoreWindow(impl->window);
        }
    }

    bool Window::setFullscreen(bool enabled)
    {
        if (!isOpen())
        {
            return false;
        }

        const Uint32 flags = enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
        return SDL_SetWindowFullscreen(impl->window, flags) == 0;
    }

    void Window::setResizable(bool enabled)
    {
        if (isOpen())
        {
            SDL_SetWindowResizable(impl->window, enabled ? SDL_TRUE : SDL_FALSE);
        }
    }

    WindowEvents& Window::events()
    {
        return impl->events;
    }

    const WindowEvents& Window::events() const
    {
        return impl->events;
    }

    void* Window::nativeHandle() const
    {
        if (!isOpen())
        {
            return nullptr;
        }

        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);

        if (SDL_GetWindowWMInfo(impl->window, &info) == SDL_FALSE)
        {
            return nullptr;
        }

#if defined(_WIN32)
        return info.info.win.window;
#elif defined(__APPLE__)
        return info.info.cocoa.window;
#elif defined(__linux__)
        return reinterpret_cast<void*>(info.info.x11.window);
#else
        return impl->window;
#endif
    }

    void Window::handlePlatformEvent(std::uint8_t event, int data1, int data2)
    {
        if (impl == nullptr)
        {
            return;
        }

        switch (event)
        {
        case SDL_WINDOWEVENT_SHOWN:
            impl->events.emit(impl->events.shownHandlers, WindowShown{});
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            impl->events.emit(impl->events.hiddenHandlers, WindowHidden{});
            break;
        case SDL_WINDOWEVENT_MOVED:
            impl->events.emit(impl->events.movedHandlers, WindowMoved{data1, data2});
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
        case SDL_WINDOWEVENT_RESIZED:
            impl->events.emit(impl->events.resizedHandlers, WindowResized{data1, data2});
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            impl->events.emit(impl->events.minimizedHandlers, WindowMinimized{});
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            impl->events.emit(impl->events.maximizedHandlers, WindowMaximized{});
            break;
        case SDL_WINDOWEVENT_RESTORED:
            impl->events.emit(impl->events.restoredHandlers, WindowRestored{});
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            impl->events.emit(impl->events.focusGainedHandlers, WindowFocusGained{});
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            impl->events.emit(impl->events.focusLostHandlers, WindowFocusLost{});
            break;
        case SDL_WINDOWEVENT_CLOSE:
            impl->events.emit(impl->events.closeRequestedHandlers, WindowCloseRequested{});
            break;
        default:
            break;
        }
    }
}
