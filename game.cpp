#include "game.h"
#include "SDL_scancode.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Tmpl8 {

void Game::Init() {}

void Game::Tick(float deltaTime) {
  if (GetKeyReleased(SDL_SCANCODE_E)) {
    entities->RegisterPlayerForward();
  }
}

void Game::Shutdown() {}
} // namespace Tmpl8
