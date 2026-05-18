#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace OctalEngine
{
    using WindowId = std::uint32_t;

    struct WindowSize
    {
        int width = 1280;
        int height = 720;
    };

    struct WindowPosition
    {
        int x = 0;
        int y = 0;
    };

    struct WindowDescriptor
    {
        std::string title = "Octal Engine";
        int width = 1280;
        int height = 720;
        int x = -1;
        int y = -1;
        bool centered = true;
        bool resizable = true;
        bool visible = true;
        bool fullscreen = false;
    };

    class WindowSubscription
    {
    public:
        WindowSubscription() = default;
        explicit WindowSubscription(std::function<void()> unsubscribe);
        ~WindowSubscription();

        WindowSubscription(const WindowSubscription&) = delete;
        WindowSubscription& operator=(const WindowSubscription&) = delete;

        WindowSubscription(WindowSubscription&& other) noexcept;
        WindowSubscription& operator=(WindowSubscription&& other) noexcept;

        void reset();
        bool active() const;

    private:
        std::function<void()> unsubscribe;
    };

    struct WindowResized
    {
        int width = 0;
        int height = 0;
    };

    struct WindowMoved
    {
        int x = 0;
        int y = 0;
    };

    struct WindowMinimized
    {
    };

    struct WindowMaximized
    {
    };

    struct WindowRestored
    {
    };

    struct WindowShown
    {
    };

    struct WindowHidden
    {
    };

    struct WindowFocusGained
    {
    };

    struct WindowFocusLost
    {
    };

    struct WindowCloseRequested
    {
    };

    class WindowEvents
    {
    public:
        WindowEvents();
        ~WindowEvents();

        WindowSubscription onResize(std::function<void(const WindowResized&)> callback);
        WindowSubscription onMove(std::function<void(const WindowMoved&)> callback);
        WindowSubscription onMinimize(std::function<void(const WindowMinimized&)> callback);
        WindowSubscription onMaximize(std::function<void(const WindowMaximized&)> callback);
        WindowSubscription onRestore(std::function<void(const WindowRestored&)> callback);
        WindowSubscription onShow(std::function<void(const WindowShown&)> callback);
        WindowSubscription onHide(std::function<void(const WindowHidden&)> callback);
        WindowSubscription onFocusGained(std::function<void(const WindowFocusGained&)> callback);
        WindowSubscription onFocusLost(std::function<void(const WindowFocusLost&)> callback);
        WindowSubscription onCloseRequested(std::function<void(const WindowCloseRequested&)> callback);

    private:
        friend class Window;

        template <typename Event>
        using Callback = std::function<void(const Event&)>;

        template <typename Event>
        struct Handler
        {
            std::size_t id = 0;
            bool active = true;
            Callback<Event> callback;
        };

        template <typename Event>
        using HandlerList = std::vector<Handler<Event>>;

        template <typename Event>
        WindowSubscription subscribe(HandlerList<Event>& handlers, Callback<Event> callback);

        template <typename Event>
        void emit(HandlerList<Event>& handlers, const Event& event);

        template <typename Event>
        void unsubscribe(HandlerList<Event>& handlers, std::size_t id);

        std::size_t nextId = 1;
        std::shared_ptr<bool> alive;
        HandlerList<WindowResized> resizedHandlers;
        HandlerList<WindowMoved> movedHandlers;
        HandlerList<WindowMinimized> minimizedHandlers;
        HandlerList<WindowMaximized> maximizedHandlers;
        HandlerList<WindowRestored> restoredHandlers;
        HandlerList<WindowShown> shownHandlers;
        HandlerList<WindowHidden> hiddenHandlers;
        HandlerList<WindowFocusGained> focusGainedHandlers;
        HandlerList<WindowFocusLost> focusLostHandlers;
        HandlerList<WindowCloseRequested> closeRequestedHandlers;
    };

    class PlatformSystem;

    class Window
    {
    public:
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        Window(Window&&) noexcept;
        Window& operator=(Window&&) noexcept;

        WindowId id() const;
        bool isOpen() const;
        void close();

        std::string title() const;
        void setTitle(const std::string& title);

        WindowSize size() const;
        void setSize(int width, int height);

        WindowPosition position() const;
        void setPosition(int x, int y);
        void center();

        void show();
        void hide();
        void maximize();
        void minimize();
        void restore();

        bool setFullscreen(bool enabled);
        void setResizable(bool enabled);

        WindowEvents& events();
        const WindowEvents& events() const;

        void* nativeHandle() const;

    private:
        friend class PlatformSystem;

        struct Impl;

        explicit Window(void* nativeWindow);
        void handlePlatformEvent(std::uint8_t event, int data1, int data2);

        std::unique_ptr<Impl> impl;
    };

    template <typename Event>
    WindowSubscription WindowEvents::subscribe(HandlerList<Event>& handlers, Callback<Event> callback)
    {
        const std::size_t id = nextId++;
        handlers.push_back(Handler<Event>{id, true, std::move(callback)});

        std::weak_ptr<bool> weakAlive = alive;

        return WindowSubscription([this, &handlers, id, weakAlive]() {
            auto locked = weakAlive.lock();

            if (locked == nullptr || !*locked)
            {
                return;
            }

            unsubscribe(handlers, id);
        });
    }

    template <typename Event>
    void WindowEvents::emit(HandlerList<Event>& handlers, const Event& event)
    {
        for (auto& handler : handlers)
        {
            if (handler.active && handler.callback)
            {
                handler.callback(event);
            }
        }

        handlers.erase(
            std::remove_if(handlers.begin(), handlers.end(), [](const Handler<Event>& handler) {
                return !handler.active;
            }),
            handlers.end());
    }

    template <typename Event>
    void WindowEvents::unsubscribe(HandlerList<Event>& handlers, std::size_t id)
    {
        for (auto& handler : handlers)
        {
            if (handler.id == id)
            {
                handler.active = false;
                break;
            }
        }
    }
}
