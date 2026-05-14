#pragma once

#include <cstdint>
#include <memory>
#include <string>

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

        void* nativeHandle() const;

    private:
        friend class PlatformSystem;

        struct Impl;

        explicit Window(void* nativeWindow);

        std::unique_ptr<Impl> impl;
    };
}
