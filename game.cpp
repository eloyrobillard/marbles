
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>

#include "game.h"
#include "surface.h"
#include "template.h"

using std::cout;
using std::endl;

namespace Tmpl8 {

void Game::Init() {}

void Game::Shutdown() {}

void Game::Tick(float deltaTime) {}

void Game::PhysicsTick(double t, double dt) {}
} // namespace Tmpl8
