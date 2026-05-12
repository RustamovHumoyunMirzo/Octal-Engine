/* Main Loop */

#include "Engine.h"
#include "EngineTime.h"
#include "Loop.h"

namespace OctalEngine
{
    Engine::Engine() {}

    Engine::~Engine() {}

    void Engine::run()
    {
        Time time;
        GameLoop loop;

        while (running)
        {
            float dt = time.step();

            loop.update(dt);
            loop.render();
        }
    }

    void Engine::stop()
    {
        running = false;
    }
}