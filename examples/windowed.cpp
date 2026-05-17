// This is a windowed sandbox for the Octal Engine.

#include "Engine.h"
#include "PlatformSystem.h"

int main()
{
    OctalEngine::PlatformSystem platform;

    OctalEngine::WindowDescriptor windowDescriptor;
    windowDescriptor.title = "Octal Engine Windowed Sandbox";
    windowDescriptor.width = 1280;
    windowDescriptor.height = 720;

    auto window = platform.createWindow(windowDescriptor);

    OctalEngine::EngineConfig config;
    config.mode = OctalEngine::WindowedMode{window.get()};

    OctalEngine::Engine engine(platform, config);

    engine.run();
}
