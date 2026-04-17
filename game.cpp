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
}

void Game::Shutdown() {}
} // namespace Tmpl8
