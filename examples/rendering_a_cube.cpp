// This sandbox submits a cube through the renderer API.

#include "Engine.h"
#include "PlatformSystem.h"
#include "Renderer.h"

#include <cstdint>
#include <vector>

namespace
{
    std::vector<OctalEngine::Vertex> createCubeVertices()
    {
        using OctalEngine::Vertex;

        return {
            {-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f},
            {1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f},
            {1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f},
            {-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f},

            {-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
            {1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
            {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},
            {-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        };
    }

    std::vector<std::uint16_t> createCubeIndices()
    {
        return {
            0, 1, 2, 2, 3, 0,
            4, 6, 5, 6, 4, 7,
            0, 4, 5, 5, 1, 0,
            3, 2, 6, 6, 7, 3,
            1, 5, 6, 6, 2, 1,
            0, 3, 7, 7, 4, 0,
        };
    }
}

int main()
{
    OctalEngine::PlatformSystem platform;

    OctalEngine::WindowDescriptor windowDescriptor;
    windowDescriptor.title = "Renderer Rotating Cube";
    windowDescriptor.width = 1280;
    windowDescriptor.height = 720;

    auto window = platform.createWindow(windowDescriptor);

    OctalEngine::EngineConfig config;
    config.mode = OctalEngine::WindowedMode{window.get()};

    OctalEngine::RendererInitSettings rendererSettings;
    rendererSettings.headless = false;
    rendererSettings.nativeWindowHandle = window->nativeHandle();
    rendererSettings.width = windowDescriptor.width;
    rendererSettings.height = windowDescriptor.height;
    rendererSettings.waitForRenderThread = true;

    OctalEngine::Renderer renderer(rendererSettings);

    if (!renderer.isInitialized())
    {
        return 1;
    }

    OctalEngine::Mesh cube(createCubeVertices(), createCubeIndices());
    OctalEngine::Engine engine(platform, config);
    float rotation = 0.0f;

    auto resizeRenderer = window->events().onResize(
        [&renderer](const OctalEngine::WindowResized& event) {
            renderer.resize(event.width, event.height);
        });

    auto renderCube = engine.events().engine().subscribe<OctalEngine::Update>(
        [&renderer, &cube, &rotation](const OctalEngine::Update& update) {
        rotation += update.dt;
        renderer.beginFrame();
        renderer.drawMesh(cube, OctalEngine::Mat4::rotationY(rotation));
        renderer.endFrame();
        });

    engine.run();
}
