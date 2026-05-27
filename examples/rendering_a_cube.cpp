// This sandbox renders a scene object through the engine's internal scene renderer.

#include "Engine.h"
#include "PlatformSystem.h"
#include "Scene.h"

#include <memory>

class CubeScene : public OctalEngine::Scene
{
public:
    CubeScene()
        : Scene("Cube Scene")
    {
    }

private:
    void onLoad() override
    {
        OctalEngine::Object camera = createObject("Camera");
        camera.addComponent<OctalEngine::TransformComponent>()->position = {0.0f, 0.0f, -6.0f};
        camera.addComponent<OctalEngine::CameraComponent>();
        setPrimaryCamera(camera);

        OctalEngine::Object cube = createObject("Cube");
        cube.addComponent<OctalEngine::TransformComponent>();
        cube.addComponent<OctalEngine::MeshGeometry>();
        cube.addComponent<OctalEngine::MeshRendererComponent>();
    }
};

int main()
{
    OctalEngine::PlatformSystem platform;

    OctalEngine::WindowDescriptor windowDescriptor;
    windowDescriptor.title = "Octal Engine Scene Cube";
    windowDescriptor.width = 1280;
    windowDescriptor.height = 720;

    auto window = platform.createWindow(windowDescriptor);

    OctalEngine::EngineConfig config;
    config.mode = OctalEngine::PlatformSystem::windowedModeFor(*window);

    OctalEngine::Engine engine(platform, config);
    engine.scenes().load(std::make_unique<CubeScene>());

    auto resizeRenderer = window->events().onResize(
        [&engine](const OctalEngine::WindowResized& event) {
            engine.resizeRenderer(event.width, event.height);
        });

    engine.run();
}
