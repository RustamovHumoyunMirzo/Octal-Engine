/* Game Loop */

#include "Loop.h"
#include "InputManager.h"

namespace OctalEngine
{
    SceneManager& GameLoop::scenes()
    {
        return sceneManager;
    }

    const SceneManager& GameLoop::scenes() const
    {
        return sceneManager;
    }

    void GameLoop::update(EventWorld& events, float dt)
    {
        sceneManager.setEventWorld(&events);
        InputManager::updateFrame(dt);
        sceneManager.update(dt);
    }

    void GameLoop::update(float dt)
    {
        InputManager::updateFrame(dt);
        sceneManager.update(dt);
    }
}
