#pragma once

#include "Events.h"
#include "Loop.h"

#include <memory>
#include <variant>

namespace OctalEngine
{
    class Platform;
    class Renderer;
    class SceneRenderer;
    class Window;

    struct HeadlessMode
    {
    };

    struct WindowedMode
    {
        Window* window = nullptr;
        void* nativeWindowHandle = nullptr;
        int width = 1280;
        int height = 720;
    };

    using RuntimeMode = std::variant<HeadlessMode, WindowedMode>;

    struct EngineConfig
    {
        RuntimeMode mode = HeadlessMode{};
        bool renderScenes = true;
    };

    class Engine
    {
    public:
        Engine();
        explicit Engine(const EngineConfig& config);
        Engine(Platform& platform, const EngineConfig& config);
        ~Engine();

        void run();
        void stop();
        EventWorld& events();
        SceneManager& scenes();
        const SceneManager& scenes() const;
        Renderer* renderer();
        const Renderer* renderer() const;
        void resizeRenderer(int width, int height);

    private:
        bool isWindowed() const;
        bool canRunFrame() const;
        void pumpPlatform();
        void initializeRendererIfNeeded();
        void renderScene();

        bool running = true;
        EngineConfig config{};
        EventWorld eventWorld;
        GameLoop gameLoop;
        Platform* platform = nullptr;
        std::unique_ptr<Renderer> internalRenderer;
        std::unique_ptr<SceneRenderer> sceneRenderer;
        int rendererWidth = 0;
        int rendererHeight = 0;
    };
}
