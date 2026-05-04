#include "game.h"

#include <windows.h>

namespace Tmpl8 {

void Game::Init() {}

void Game::Tick(float deltaTime) {
  if (GetKey(SDL_SCANCODE_RIGHT)) {
    entities->RegisterPlayerRight(deltaTime);
  }

  if (GetKey(SDL_SCANCODE_LEFT)) {
    entities->RegisterPlayerLeft(deltaTime);
  }

  if (GetKeyPressed(SDL_SCANCODE_SPACE)) {
    Restart();
  }

  if (entities->GetDynamicEntities()[0].body.position.x > 200.0f) {
    renderer->ShowVictoryMessage();
  }
}

void Game::Shutdown() {}

void Game::Restart() {
  entities->Restart();
  camera->Restart(entities->ProvideCameraFollow());
  renderer->Restart();
}
} // namespace Tmpl8
