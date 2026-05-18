#pragma once

#include <array>
#include <cmath>

namespace OctalEngine
{
    struct Mat4
    {
        std::array<float, 16> values{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f};

        static Mat4 identity()
        {
            return {};
        }

        static Mat4 rotationY(float radians)
        {
            const float c = std::cos(radians);
            const float s = std::sin(radians);

            Mat4 result;
            result.values = {
                c, 0.0f, -s, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                s, 0.0f, c, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f};
            return result;
        }
    };
}
