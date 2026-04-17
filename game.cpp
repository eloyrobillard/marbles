#include "game.h"
#include "SDL_scancode.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Tmpl8 {

void Game::Init() {}

void Game::Tick(float deltaTime) {
  if (GetKeyReleased(SDL_SCANCODE_RIGHT)) {
    entities->RegisterPlayerRight();
  }

  if (GetKeyReleased(SDL_SCANCODE_LEFT)) {
    entities->RegisterPlayerLeft();
  }

  if (GetKeyReleased(SDL_SCANCODE_SPACE)) {
    Restart();
  }
}

void Game::Shutdown() {}

void Game::Restart() {
  entities->Restart();
  camera->Restart(entities->ProvideCameraFollow());
}
} // namespace Tmpl8
