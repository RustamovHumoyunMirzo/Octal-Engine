#pragma once

#include "RendererMath.h"
#include "Mesh.h"

#include <memory>
#include <span>
#include <vector>

namespace OctalEngine
{
    /// Supported renderer types
    enum class RendererType
    {
        Auto,
        Direct3D11,  ///< DirectX 11
        Direct3D12,  ///< DirectX 12
        OpenGL,      ///< OpenGL
        OpenGLES,    ///< OpenGL ES
        Vulkan,      ///< Vulkan
        Metal,       ///< Metal
        WebGPU,      ///< WebGPU
    };

    struct RendererInitSettings
    {
        bool headless = true;
        void* nativeWindowHandle = nullptr;
        int width = 1280;
        int height = 720;
        bool waitForRenderThread = true;
        RendererType rendererType = RendererType::Auto;  ///< Renderer API to use, defaults to auto selection
    };

    struct RenderCamera
    {
        float eyeX = 0.0f;
        float eyeY = 0.0f;
        float eyeZ = -6.0f;
        float targetX = 0.0f;
        float targetY = 0.0f;
        float targetZ = 0.0f;
        float upX = 0.0f;
        float upY = 1.0f;
        float upZ = 0.0f;
        float fov = 60.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        bool isOrthographic = false;
    };

    struct RenderColor
    {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;
    };

    class Renderer
    {
    public:
        Renderer();
        explicit Renderer(const RendererInitSettings& settings);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        Renderer(Renderer&&) noexcept;
        Renderer& operator=(Renderer&&) noexcept;

        bool initialize(const RendererInitSettings& settings = {});
        void shutdown();

        void beginFrame();
        void setCamera(const RenderCamera& camera);
        void drawMesh(const Mesh& mesh, const Mat4& transform = Mat4::identity(), const RenderColor& color = {});
        void drawVertices(std::span<const Vertex> vertices, const Mat4& transform = Mat4::identity());
        void drawVertices(const std::vector<Vertex>& vertices, const Mat4& transform = Mat4::identity());
        void resize(int width, int height);
        void endFrame();

        bool isInitialized() const;
        bool isHeadless() const;

        /// Returns the actual renderer type being used
        /// If not initialized or in Auto mode, returns the actual backend type selected
        RendererType getRendererType() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl;
    };
}
