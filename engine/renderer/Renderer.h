#pragma once

#include "RendererMath.h"
#include "Mesh.h"

#include <memory>
#include <span>
#include <vector>

namespace OctalEngine
{
    struct RendererInitSettings
    {
        bool headless = true;
        void* nativeWindowHandle = nullptr;
        int width = 1280;
        int height = 720;
        bool waitForRenderThread = true;
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
        void drawMesh(const Mesh& mesh, const Mat4& transform = Mat4::identity());
        void drawVertices(std::span<const Vertex> vertices, const Mat4& transform = Mat4::identity());
        void drawVertices(const std::vector<Vertex>& vertices, const Mat4& transform = Mat4::identity());
        void resize(int width, int height);
        void endFrame();

        bool isInitialized() const;
        bool isHeadless() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl;
    };
}
