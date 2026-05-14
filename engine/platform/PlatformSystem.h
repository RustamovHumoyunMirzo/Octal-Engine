#pragma once

#include "Platform.h"
#include "Window.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace OctalEngine
{
    class PlatformSystem final : public Platform
    {
    public:
        PlatformSystem();
        ~PlatformSystem() override;

        PlatformSystem(const PlatformSystem&) = delete;
        PlatformSystem& operator=(const PlatformSystem&) = delete;

        std::shared_ptr<Window> createWindow(const WindowDescriptor& descriptor = {});
        std::shared_ptr<Window> findWindow(WindowId id) const;
        void closeWindow(WindowId id);
        void closeAllWindows();
        std::size_t windowCount() const;

        void pumpEvents() override;
        bool shouldQuit() const override;
        void requestQuit() override;
        void clearQuitRequest();

        bool quitWhenLastWindowClosed() const;
        void setQuitWhenLastWindowClosed(bool enabled);

    private:
        void pruneClosedWindows();

        std::vector<std::weak_ptr<Window>> windows;
        bool quitRequested = false;
        bool quitOnLastWindowClosed = true;
    };
}
