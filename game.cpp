
#include <fstream>
#include <ios>
#include <stdio.h>
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>

#include "game.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "surface.h"
#include "template.h"

using std::cout;
using std::endl;

namespace Tmpl8 {

void Game::Init() {
  std::ifstream ifs;
  ifs.open("assets/basic_ramp.gpmesh");

  if (!ifs.is_open()) {
    std::cerr << "Could not open file" << std::endl;
    Game::Shutdown();
  }

  std::string gpmesh;
  std::string line;

  while (!ifs.eof()) {
    std::getline(ifs, line);
    gpmesh.append(line);
  }

  rapidjson::Document d;
  d.Parse(gpmesh.c_str());

  ifs.close();
}

void Game::Shutdown() {}

void Game::Tick(float deltaTime) {}

void Game::PhysicsTick(double t, double dt) {}
} // namespace Tmpl8
