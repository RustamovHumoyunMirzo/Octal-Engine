// This is a headless sandbox for the Octal Engine.

#include "Engine.h"

int main()
{
    OctalEngine::EngineConfig config;
    config.mode = OctalEngine::HeadlessMode{};

    OctalEngine::Engine engine(config);

    engine.run();
}
