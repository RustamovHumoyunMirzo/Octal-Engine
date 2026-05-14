#pragma once

#include <variant>

namespace OctalEngine
{
    class Platform;
    class Window;

    struct HeadlessMode
    {
    };

    struct WindowedMode
    {
        Window* window = nullptr;
    };

    using RuntimeMode = std::variant<HeadlessMode, WindowedMode>;

    struct EngineConfig
    {
        RuntimeMode mode = HeadlessMode{};
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

    private:
        bool isWindowed() const;
        bool canRunFrame() const;
        void pumpPlatform();

        bool running = true;
        EngineConfig config{};
        Platform* platform = nullptr;
    };
}
