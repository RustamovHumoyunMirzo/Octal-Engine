#pragma once

#include <memory>

namespace OctalEngine::RendererInternal
{
    struct CommandBuffer;
}

namespace OctalEngine::RendererBackend
{
    /// Backend renderer types
    enum class RendererType
    {
        Auto,        ///< Let the backend choose automatically
        Direct3D11,  ///< DirectX 11
        Direct3D12,  ///< DirectX 12
        OpenGL,      ///< OpenGL
        OpenGLES,    ///< OpenGL ES
        Vulkan,      ///< Vulkan
        Metal,       ///< Metal
        WebGPU,      ///< WebGPU
    };

    struct BackendSettings
    {
        bool headless = true;
        void* nativeWindowHandle = nullptr;
        int width = 1280;
        int height = 720;
        RendererType rendererType = RendererType::Auto;  ///< Renderer API to use
    };

    class IRendererBackend
    {
    public:
        virtual ~IRendererBackend() = default;

        virtual bool initialize(const BackendSettings& settings) = 0;
        virtual void shutdown() = 0;
        virtual void execute(const RendererInternal::CommandBuffer& commands) = 0;

        /// Returns the actual renderer type being used (may differ from requested type in Auto mode)
        virtual RendererType getRendererType() const = 0;
    };

    std::unique_ptr<IRendererBackend> createRendererBackend();
}
