#include "Renderer.h"

#include "backend/IRendererBackend.h"
#include "internal/RenderCommands.h"
#include "internal/RenderThread.h"

#include <stdexcept>

namespace OctalEngine
{
    namespace
    {
        RendererBackend::RendererType convertRendererType(RendererType type)
        {
            switch (type)
            {
            case RendererType::Auto:
                return RendererBackend::RendererType::Auto;
            case RendererType::Direct3D11:
                return RendererBackend::RendererType::Direct3D11;
            case RendererType::Direct3D12:
                return RendererBackend::RendererType::Direct3D12;
            case RendererType::OpenGL:
                return RendererBackend::RendererType::OpenGL;
            case RendererType::OpenGLES:
                return RendererBackend::RendererType::OpenGLES;
            case RendererType::Vulkan:
                return RendererBackend::RendererType::Vulkan;
            case RendererType::Metal:
                return RendererBackend::RendererType::Metal;
            case RendererType::WebGPU:
                return RendererBackend::RendererType::WebGPU;
            default:
                return RendererBackend::RendererType::Auto;
            }
        }

        RendererType convertFromBackendRendererType(RendererBackend::RendererType type)
        {
            switch (type)
            {
            case RendererBackend::RendererType::Auto:
                return RendererType::Auto;
            case RendererBackend::RendererType::Direct3D11:
                return RendererType::Direct3D11;
            case RendererBackend::RendererType::Direct3D12:
                return RendererType::Direct3D12;
            case RendererBackend::RendererType::OpenGL:
                return RendererType::OpenGL;
            case RendererBackend::RendererType::OpenGLES:
                return RendererType::OpenGLES;
            case RendererBackend::RendererType::Vulkan:
                return RendererType::Vulkan;
            case RendererBackend::RendererType::Metal:
                return RendererType::Metal;
            case RendererBackend::RendererType::WebGPU:
                return RendererType::WebGPU;
            default:
                return RendererType::Auto;
            }
        }
    }

    struct Renderer::Impl
    {
        RendererInitSettings settings{};
        RendererInternal::CommandBuffer writeBuffer;
        RendererInternal::RenderThread renderThread;
        bool initialized = false;
        bool initializationFailed = false;
        bool frameOpen = false;
    };

    Renderer::Renderer()
        : impl(std::make_unique<Impl>())
    {
    }

    Renderer::Renderer(const RendererInitSettings& settings)
        : Renderer()
    {
        initialize(settings);
    }

    Renderer::~Renderer()
    {
        shutdown();
    }

    Renderer::Renderer(Renderer&&) noexcept = default;
    Renderer& Renderer::operator=(Renderer&&) noexcept = default;

    bool Renderer::initialize(const RendererInitSettings& settings)
    {
        if (impl->initialized)
        {
            return true;
        }

        impl->settings = settings;
        impl->initializationFailed = false;

        RendererBackend::BackendSettings backendSettings;
        backendSettings.headless = settings.headless;
        backendSettings.nativeWindowHandle = settings.nativeWindowHandle;
        backendSettings.width = settings.width;
        backendSettings.height = settings.height;
        backendSettings.rendererType = convertRendererType(settings.rendererType);

        auto backend = RendererBackend::createRendererBackend();
        if (!impl->renderThread.start(std::move(backend), backendSettings))
        {
            impl->initializationFailed = true;
            return false;
        }

        impl->initialized = true;
        return true;
    }

    void Renderer::shutdown()
    {
        if (impl == nullptr || !impl->initialized)
        {
            return;
        }

        if (impl->frameOpen)
        {
            endFrame();
        }

        impl->renderThread.stop();
        impl->writeBuffer.clear();
        impl->initialized = false;
    }

    void Renderer::beginFrame()
    {
        if (!impl->initialized)
        {
            if (impl->initializationFailed || !initialize())
            {
                return;
            }
        }

        impl->writeBuffer.clear();
        impl->frameOpen = true;
    }

    void Renderer::drawMesh(const Mesh& mesh, const Mat4& transform)
    {
        if (!impl->frameOpen || mesh.empty())
        {
            return;
        }

        impl->writeBuffer.commands.emplace_back(RendererInternal::DrawMeshCommand{mesh, transform});
    }

    void Renderer::drawVertices(std::span<const Vertex> vertices, const Mat4& transform)
    {
        if (!impl->frameOpen || vertices.empty())
        {
            return;
        }

        std::vector<Vertex> copy(vertices.begin(), vertices.end());
        impl->writeBuffer.commands.emplace_back(RendererInternal::DrawVerticesCommand{std::move(copy), transform});
    }

    void Renderer::drawVertices(const std::vector<Vertex>& vertices, const Mat4& transform)
    {
        drawVertices(std::span<const Vertex>(vertices.data(), vertices.size()), transform);
    }

    void Renderer::resize(int width, int height)
    {
        if (!impl->initialized || width <= 0 || height <= 0)
        {
            return;
        }

        RendererInternal::CommandBuffer submitted;
        submitted.commands.emplace_back(RendererInternal::ResizeCommand{width, height});
        impl->renderThread.submit(std::move(submitted), impl->settings.waitForRenderThread);
    }

    void Renderer::endFrame()
    {
        if (!impl->initialized || !impl->frameOpen)
        {
            return;
        }

        RendererInternal::CommandBuffer submitted;
        submitted.commands.swap(impl->writeBuffer.commands);
        impl->renderThread.submit(std::move(submitted), impl->settings.waitForRenderThread);
        impl->frameOpen = false;
    }

    bool Renderer::isInitialized() const
    {
        return impl != nullptr && impl->initialized;
    }

    bool Renderer::isHeadless() const
    {
        return impl == nullptr || impl->settings.headless;
    }

    RendererType Renderer::getRendererType() const
    {
        if (impl == nullptr || !impl->initialized)
        {
            return RendererType::Auto;
        }
        return convertFromBackendRendererType(impl->renderThread.getRendererType());
    }
}
