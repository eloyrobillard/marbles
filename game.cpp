#include "game.h"

#include <windows.h>

namespace Tmpl8 {

int checkpointID = 0;
vector<vector<vec3>> checkpoints{
    {{9.826279640197754, -0.024219999089837074, 0.1309020072221756}},
    {{53.835, -.45012, -3.5062}}};

void Game::Init() {}

void Game::Tick(float deltaTime) {
  if (GetKey(SDL_SCANCODE_RIGHT)) {
    entities->RegisterInputRight(deltaTime);
  }

  if (GetKey(SDL_SCANCODE_LEFT)) {
    entities->RegisterInputLeft(deltaTime);
  }

  if (GetKeyPressed(SDL_SCANCODE_SPACE)) {
    Restart();
  }

  if (entities->GetDynamicEntities()[0].GetPositionAsRef().x > 55.0f) {
    checkpointID = 1;
  }

  if (entities->GetDynamicEntities()[0].GetPositionAsRef().x > 200.0f) {
    renderer->ShowVictoryMessage();
    checkpointID = 0;
  }
}

void Game::Shutdown() {}

void Game::Restart() {
  const auto &dynamicEntitiesPos = checkpoints[checkpointID];
  entities->ToCheckpoint(dynamicEntitiesPos);
  camera->Restart(entities->ProvideCameraFollow());
  renderer->Restart();
}
} // namespace Tmpl8
