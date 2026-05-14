#pragma once

namespace OctalEngine
{
    class Platform
    {
    public:
        virtual ~Platform() = default;

        virtual void pumpEvents() = 0;
        virtual bool shouldQuit() const = 0;
        virtual void requestQuit() = 0;
    };
}
