#pragma once

namespace OctalEngine
{
    class Scene;
    class Renderer;

    class SceneRenderer
    {
    public:
        SceneRenderer() = default;
        void render(Scene* scene, Renderer* renderer);
    };
}
