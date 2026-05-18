#pragma once

#include <cstdint>

namespace OctalEngine
{
    class Material
    {
    public:
        std::uint64_t id() const
        {
            return materialId;
        }

    private:
        std::uint64_t materialId = 0;
    };
}
