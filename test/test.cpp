#include "Engine.h"
#include "InputManager.h"
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
    windowDescriptor.title = "Octal Engine Input + Renderer Sandbox";
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

    OctalEngine::Renderer renderer(rendererSettings);
    if (!renderer.isInitialized())
    {
        return 1;
    }

    OctalEngine::InputManager::defineAction("Move", OctalEngine::InputValueType::Axis2D);
    OctalEngine::InputManager::defineAction("Shoot");
    OctalEngine::InputManager::defineAction("Quit");
    OctalEngine::InputManager::defineAction("RebindShoot");
    OctalEngine::InputManager::addDefaultBinding("Gameplay", "Move", OctalEngine::InputBinding::key(OctalEngine::Key::A, -1.0f, 0.0f));
    OctalEngine::InputManager::addDefaultBinding("Gameplay", "Move", OctalEngine::InputBinding::key(OctalEngine::Key::D, 1.0f, 0.0f));
    OctalEngine::InputManager::addDefaultBinding("Gameplay", "Move", OctalEngine::InputBinding::key(OctalEngine::Key::W, 0.0f, 1.0f));
    OctalEngine::InputManager::addDefaultBinding("Gameplay", "Move", OctalEngine::InputBinding::key(OctalEngine::Key::S, 0.0f, -1.0f));
    OctalEngine::InputManager::addDefaultBinding("Gameplay", "Shoot", OctalEngine::InputBinding::mouseButton(OctalEngine::MouseButton::Left));
    OctalEngine::InputManager::addDefaultBinding("Gameplay", "Quit", OctalEngine::InputBinding::key(OctalEngine::Key::Escape));
    OctalEngine::InputManager::addDefaultBinding("Gameplay", "RebindShoot", OctalEngine::InputBinding::key(OctalEngine::Key::F1));
    OctalEngine::InputManager::pushContext({"Gameplay", 0, true});

    OctalEngine::Mesh cube(createCubeVertices(), createCubeIndices());
    OctalEngine::Engine engine(platform, config);
    float rotation = 0.0f;
    bool shootRebound = false;

    auto resizeRenderer = window->events().onResize(
        [&renderer](const OctalEngine::WindowResized& event) {
            renderer.resize(event.width, event.height);
        });

    auto inputEvents = OctalEngine::InputManager::onAction(
        [&engine, &window, &shootRebound](const OctalEngine::InputActionEvent& event) {
            if (event.action == "Quit" && event.pressed)
            {
                engine.stop();
            }

            if (event.action == "Shoot" && event.pressed)
            {
                window->setTitle("Shoot action pressed");
            }

            if (event.action == "RebindShoot" && event.pressed)
            {
                OctalEngine::InputManager::clearBindings("Gameplay", "Shoot");
                OctalEngine::InputManager::addBinding(
                    "Gameplay",
                    "Shoot",
                    shootRebound
                        ? OctalEngine::InputBinding::mouseButton(OctalEngine::MouseButton::Left)
                        : OctalEngine::InputBinding::key(OctalEngine::Key::Space));
                shootRebound = !shootRebound;
                window->setTitle(shootRebound ? "Shoot rebound to Space" : "Shoot rebound to Mouse Left");
            }
        });

    auto renderCube = engine.events().engine().subscribe<OctalEngine::Update>(
        [&renderer, &cube, &rotation](const OctalEngine::Update& update) {
            const OctalEngine::InputValue move = OctalEngine::InputManager::actionValue("Move");
            rotation += update.dt * (1.0f + move.x);

            renderer.beginFrame();
            renderer.drawMesh(cube, OctalEngine::Mat4::rotationY(rotation));
            renderer.endFrame();
        });

    engine.run();
}
