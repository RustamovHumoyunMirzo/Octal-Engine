#pragma once

namespace OctalEngine
{
    class Engine
    {
    public:
        Engine();
        ~Engine();

        void run();
        void stop();

    private:
        bool running = true;
    };
}