#include "Window.h"

#include <SDL2/SDL.h>

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
    };

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

    void* Window::nativeHandle() const
    {
        return isOpen() ? impl->window : nullptr;
    }
}
