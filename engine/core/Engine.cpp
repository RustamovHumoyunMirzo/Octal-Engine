/* Main Loop */

#include "Engine.h"
#include "EngineTime.h"
#include "Loop.h"
#include "Platform.h"

namespace OctalEngine
{
    Engine::Engine() {}

    Engine::Engine(const EngineConfig& config)
        : config(config)
    {
    }

    Engine::Engine(Platform& platform, const EngineConfig& config)
        : config(config), platform(&platform)
    {
    }

    Engine::~Engine() {}

    void Engine::run()
    {
        Time time;
        GameLoop loop;

        while (running)
        {
            pumpPlatform();

            if (!canRunFrame())
            {
                stop();
                break;
            }

            const float dt = time.step();

            eventWorld.engine().emit<Update>({dt});
            loop.update(dt);
            eventWorld.flush();
        }
    }

    void Engine::stop()
    {
        running = false;
    }

    EventWorld& Engine::events()
    {
        return eventWorld;
    }

    bool Engine::isWindowed() const
    {
        return std::holds_alternative<WindowedMode>(config.mode);
    }

    bool Engine::canRunFrame() const
    {
        if (platform != nullptr && platform->shouldQuit())
        {
            return false;
        }

        if (!isWindowed())
        {
            return true;
        }

        const auto& mode = std::get<WindowedMode>(config.mode);
        return mode.window != nullptr;
    }

    void Engine::pumpPlatform()
    {
        if (platform != nullptr)
        {
            platform->pumpEvents();
        }
    }
}
