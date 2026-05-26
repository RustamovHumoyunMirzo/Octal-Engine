#pragma once

#include "Events.h"
#include "Scene.h"

namespace OctalEngine
{
    class GameLoop
    {
    public:
        SceneManager& scenes();
        const SceneManager& scenes() const;

        void update(EventWorld& events, float dt);

        void update(float dt);

    private:
        SceneManager sceneManager;
    };
}
