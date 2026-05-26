#pragma once

#include "Engine.h"
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

        static WindowedMode windowedModeFor(Window& window);

    private:
        void pruneClosedWindows();
        void openGameController(int deviceIndex);
        void closeGameController(int instanceId);
        void closeGameControllers();

        std::vector<std::weak_ptr<Window>> windows;
        std::vector<void*> gameControllers;
        bool quitRequested = false;
        bool quitOnLastWindowClosed = true;
    };
}
