#pragma once

#include <memory>

namespace OctalEngine::RendererInternal
{
    struct CommandBuffer;
}

namespace OctalEngine::RendererBackend
{
    struct BackendSettings
    {
        bool headless = true;
        void* nativeWindowHandle = nullptr;
        int width = 1280;
        int height = 720;
    };

    class IRendererBackend
    {
    public:
        virtual ~IRendererBackend() = default;

        virtual bool initialize(const BackendSettings& settings) = 0;
        virtual void shutdown() = 0;
        virtual void execute(const RendererInternal::CommandBuffer& commands) = 0;
    };

    std::unique_ptr<IRendererBackend> createRendererBackend();
}
