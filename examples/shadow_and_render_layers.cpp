// Example demonstrating dynamic shadows, sorting order, and render layers.
// This shows how castShadows, receiveShadows, sortingOrder, and renderLayer work together.

#include "Engine.h"
#include "PlatformSystem.h"
#include "Scene.h"

#include <memory>
#include <cmath>

class ShadowRenderLayerScene : public OctalEngine::Scene
{
public:
    ShadowRenderLayerScene()
        : Scene("Shadow and Render Layers Scene")
    {
    }

private:
    void onLoad() override
    {
        // Setup camera
        OctalEngine::Object camera = createObject("Camera");
        camera.addComponent<OctalEngine::TransformComponent>()->position = {5.0f, 6.0f, -8.0f};
        camera.addComponent<OctalEngine::CameraComponent>();
        setPrimaryCamera(camera);

        // Setup directional light that casts shadows
        OctalEngine::Object sun = createObject("Sun");
        auto* sunTransform = sun.addComponent<OctalEngine::TransformComponent>();
        sunTransform->rotation = {0.35f, 0.0f, 0.0f, 0.94f}; // Tilted down light
        auto* light = sun.addComponent<OctalEngine::LightComponent>();
        light->type = OctalEngine::LightType::Directional;
        light->intensity = 2.5f;
        light->castShadows = true;
        light->shadowStrength = 1.0f;

        // Ground plane - receives shadows but doesn't cast them
        OctalEngine::Object ground = createObject("Ground");
        auto* groundTransform = ground.addComponent<OctalEngine::TransformComponent>();
        groundTransform->position = {0.0f, 0.0f, 0.0f};
        groundTransform->scale = {10.0f, 0.5f, 10.0f};
        auto* groundGeometry = ground.addComponent<OctalEngine::MeshGeometry>();
        groundGeometry->primitive = OctalEngine::PrimitiveType::Cube;
        auto* groundRenderer = ground.addComponent<OctalEngine::MeshRendererComponent>();
        groundRenderer->castShadows = false; // Ground doesn't cast shadows
        groundRenderer->receiveShadows = true; // But receives them
        groundRenderer->renderLayer = 0; // Background layer
        groundRenderer->sortingOrder = 0;

        // Cube 1 - casts shadows, receives shadows, layer 1
        OctalEngine::Object cube1 = createObject("Cube1");
        auto* cube1Transform = cube1.addComponent<OctalEngine::TransformComponent>();
        cube1Transform->position = {-2.0f, 1.0f, 2.0f};
        cube1Transform->scale = {0.8f, 0.8f, 0.8f};
        auto* cube1Geometry = cube1.addComponent<OctalEngine::MeshGeometry>();
        cube1Geometry->primitive = OctalEngine::PrimitiveType::Cube;
        auto* cube1Renderer = cube1.addComponent<OctalEngine::MeshRendererComponent>();
        cube1Renderer->castShadows = true;
        cube1Renderer->receiveShadows = true;
        cube1Renderer->renderLayer = 1; // Render layer 1
        cube1Renderer->sortingOrder = 0;

        // Cube 2 - casts shadows, receives shadows, layer 1, different sorting order
        OctalEngine::Object cube2 = createObject("Cube2");
        auto* cube2Transform = cube2.addComponent<OctalEngine::TransformComponent>();
        cube2Transform->position = {2.0f, 1.0f, 2.0f};
        cube2Transform->scale = {0.6f, 0.6f, 0.6f};
        auto* cube2Geometry = cube2.addComponent<OctalEngine::MeshGeometry>();
        cube2Geometry->primitive = OctalEngine::PrimitiveType::Cube;
        auto* cube2Renderer = cube2.addComponent<OctalEngine::MeshRendererComponent>();
        cube2Renderer->castShadows = true;
        cube2Renderer->receiveShadows = true;
        cube2Renderer->renderLayer = 1; // Same layer as cube1
        cube2Renderer->sortingOrder = 1; // But rendered after cube1

        // Sphere - casts shadows, receives shadows, layer 2
        OctalEngine::Object sphere = createObject("Sphere");
        auto* sphereTransform = sphere.addComponent<OctalEngine::TransformComponent>();
        sphereTransform->position = {0.0f, 2.0f, -2.0f};
        sphereTransform->scale = {1.0f, 1.0f, 1.0f};
        auto* sphereGeometry = sphere.addComponent<OctalEngine::MeshGeometry>();
        sphereGeometry->primitive = OctalEngine::PrimitiveType::Sphere;
        auto* sphereRenderer = sphere.addComponent<OctalEngine::MeshRendererComponent>();
        sphereRenderer->castShadows = true;
        sphereRenderer->receiveShadows = true;
        sphereRenderer->renderLayer = 2; // Foreground layer
        sphereRenderer->sortingOrder = 0;

        // Static object - doesn't cast shadows but receives them
        OctalEngine::Object staticCube = createObject("StaticCube");
        auto* staticTransform = staticCube.addComponent<OctalEngine::TransformComponent>();
        staticTransform->position = {-3.0f, 1.5f, -3.0f};
        staticTransform->scale = {1.0f, 1.0f, 1.0f};
        auto* staticGeometry = staticCube.addComponent<OctalEngine::MeshGeometry>();
        staticGeometry->primitive = OctalEngine::PrimitiveType::Cube;
        auto* staticRenderer = staticCube.addComponent<OctalEngine::MeshRendererComponent>();
        staticRenderer->castShadows = false; // Static object doesn't cast shadows
        staticRenderer->receiveShadows = true; // But receives them
        staticRenderer->renderLayer = 1;
        staticRenderer->sortingOrder = 0;
    }

    void onUpdate(float dt) override
    {
        time += dt;
        // Optional: Add animated rotation to objects here
    }

    float time = 0.0f;
};

int main()
{
    OctalEngine::PlatformSystem platform;

    OctalEngine::WindowDescriptor windowDescriptor;
    windowDescriptor.title = "Octal Engine - Shadow and Render Layers";
    windowDescriptor.width = 1280;
    windowDescriptor.height = 720;

    auto window = platform.createWindow(windowDescriptor);

    OctalEngine::EngineConfig config;
    config.mode = OctalEngine::PlatformSystem::windowedModeFor(*window);

    OctalEngine::Engine engine(platform, config);
    engine.scenes().load(std::make_unique<ShadowRenderLayerScene>());

    auto resizeRenderer = window->events().onResize(
        [&engine](const OctalEngine::WindowResized& event) {
            engine.resizeRenderer(event.width, event.height);
        });

    engine.run();
}
