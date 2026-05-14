#include "PlatformSystem.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <stdexcept>

namespace OctalEngine
{
    PlatformSystem::PlatformSystem()
    {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
        {
            throw std::runtime_error(SDL_GetError());
        }
    }

    PlatformSystem::~PlatformSystem()
    {
        closeAllWindows();
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
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
        SDL_Event event;

        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
            {
                requestQuit();
                continue;
            }

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                closeWindow(event.window.windowID);
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
}
