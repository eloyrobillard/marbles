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
}

void Game::Shutdown() {}

void Game::Restart() {
  entities->Restart();
  camera->Restart(entities->ProvideCameraFollow());
}
} // namespace Tmpl8
