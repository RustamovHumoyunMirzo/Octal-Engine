/* Game Loop */

#include "Loop.h"
#include "InputManager.h"

namespace OctalEngine
{
    void GameLoop::update(float dt)
    {
        InputManager::updateFrame(dt);
    }
}
