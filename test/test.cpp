#include "Engine.h"
#include "InputManager.h"
#include "PlatformSystem.h"
#include "Scene.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>

class CubeControlsScene : public OctalEngine::Scene
{
public:
    CubeControlsScene()
        : Scene("Cube Controls")
    {
    }

private:
    void onLoad() override
    {
        OctalEngine::Object camera = createObject("Camera");
        camera.addComponent<OctalEngine::TransformComponent>()->position = {0.0f, 4.0f, -10.0f};
        camera.addComponent<OctalEngine::CameraComponent>();
        setPrimaryCamera(camera);
        assert(hasPrimaryCamera());

        sun = createObject("Sun");
        auto* sunTransform = sun.addComponent<OctalEngine::TransformComponent>();
        sunTransform->rotation = lightRotation(lightPitch);
        auto* light = sun.addComponent<OctalEngine::LightComponent>();
        light->type = OctalEngine::LightType::Directional;
        light->intensity = 2.5f;
        light->castShadows = true;

        ground = createObject("Ground");
        auto* groundTransform = ground.addComponent<OctalEngine::TransformComponent>();
        groundTransform->position = {0.0f, -1.0f, 2.0f};
        groundTransform->scale = {8.0f, 1.0f, 8.0f};
        auto* groundRenderer = ground.addComponent<OctalEngine::MeshRendererComponent>();
        groundRenderer->primitive = OctalEngine::PrimitiveType::Plane;
        groundRenderer->castShadows = false;

        cube = createObject("Player Cube");
        auto* cubeTransform = cube.addComponent<OctalEngine::TransformComponent>();
        cubeTransform->position = cubePosition;
        cubeTransform->scale = {0.75f, 0.75f, 0.75f};
        auto* cubeRenderer = cube.addComponent<OctalEngine::MeshRendererComponent>();
        cubeRenderer->primitive = OctalEngine::PrimitiveType::Cube;
        cubeRenderer->castShadows = true;
    }

    void onUpdate(float dt) override
    {
        const OctalEngine::InputValue move = OctalEngine::InputManager::actionValue("Move");
        const float speed = 4.0f;

        cubePosition.x += move.x * speed * dt;
        cubePosition.z += move.y * speed * dt;

        if (OctalEngine::InputManager::wasActionPressedThisFrame("Jump") && grounded)
        {
            verticalVelocity = 5.5f;
            grounded = false;
        }

        verticalVelocity -= 12.0f * dt;
        cubePosition.y += verticalVelocity * dt;

        if (cubePosition.y <= 0.0f)
        {
            cubePosition.y = 0.0f;
            verticalVelocity = 0.0f;
            grounded = true;
        }

        rotation += dt * (1.0f + std::abs(move.x) + std::abs(move.y));
        lightTime += dt;
        lightPitch = -0.75f + std::sin(lightTime) * 0.45f;

        if (auto* transform = cube.getComponent<OctalEngine::TransformComponent>())
        {
            const float halfAngle = rotation * 0.5f;
            transform->position = cubePosition;
            transform->rotation = {0.0f, std::sin(halfAngle), 0.0f, std::cos(halfAngle)};
        }

        if (auto* transform = sun.getComponent<OctalEngine::TransformComponent>())
        {
            transform->rotation = lightRotation(lightPitch);
        }
    }

    OctalEngine::Quaternion lightRotation(float pitch) const
    {
        constexpr float yaw = 0.55f;
        const float halfPitch = pitch * 0.5f;
        const float halfYaw = yaw * 0.5f;
        return {
            std::sin(halfPitch) * std::cos(halfYaw),
            std::cos(halfPitch) * std::sin(halfYaw),
            -std::sin(halfPitch) * std::sin(halfYaw),
            std::cos(halfPitch) * std::cos(halfYaw)};
    }

    OctalEngine::Object cube;
    OctalEngine::Object ground;
    OctalEngine::Object sun;
    OctalEngine::Vec3 cubePosition{0.0f, 0.0f, 0.0f};
    float verticalVelocity = 0.0f;
    float rotation = 0.0f;
    float lightTime = 0.0f;
    float lightPitch = -0.75f;
    bool grounded = true;
};

int main()
{
    OctalEngine::PlatformSystem platform;

    OctalEngine::WindowDescriptor windowDescriptor;
    windowDescriptor.title = "Octal Engine Cube Controls";
    windowDescriptor.width = 1280;
    windowDescriptor.height = 720;

    auto window = platform.createWindow(windowDescriptor);

    OctalEngine::EngineConfig config;
    config.mode = OctalEngine::PlatformSystem::windowedModeFor(*window);

    OctalEngine::InputManager::defineAction("Move", OctalEngine::InputValueType::Axis2D);
    OctalEngine::InputManager::defineAction("Jump");
    OctalEngine::InputManager::defineAction("Quit");
    OctalEngine::InputManager::addDefaultBinding("Game", "Move", OctalEngine::InputBinding::key(OctalEngine::Key::A, -1.0f, 0.0f));
    OctalEngine::InputManager::addDefaultBinding("Game", "Move", OctalEngine::InputBinding::key(OctalEngine::Key::D, 1.0f, 0.0f));
    OctalEngine::InputManager::addDefaultBinding("Game", "Move", OctalEngine::InputBinding::key(OctalEngine::Key::W, 0.0f, 1.0f));
    OctalEngine::InputManager::addDefaultBinding("Game", "Move", OctalEngine::InputBinding::key(OctalEngine::Key::S, 0.0f, -1.0f));
    OctalEngine::InputManager::addDefaultBinding("Game", "Jump", OctalEngine::InputBinding::key(OctalEngine::Key::Space));
    OctalEngine::InputManager::addDefaultBinding("Game", "Quit", OctalEngine::InputBinding::key(OctalEngine::Key::Escape));
    OctalEngine::InputManager::pushContext({"Game", 0, true});

    OctalEngine::Engine engine(platform, config);
    engine.scenes().load(std::make_unique<CubeControlsScene>());

    auto quitOnEscape = OctalEngine::InputManager::onAction(
        [&engine](const OctalEngine::InputActionEvent& event) {
            if (event.action == "Quit" && event.pressed)
            {
                engine.stop();
            }
        });

    auto resizeRenderer = window->events().onResize(
        [&engine](const OctalEngine::WindowResized& event) {
            engine.resizeRenderer(event.width, event.height);
        });

    engine.run();
}
