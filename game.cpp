
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>

#include "game.h"
#include "rapidjson/document.h"
#include "template.h"
#include "utils.h"

using std::cerr;
using std::cout;
using std::endl;
using namespace rapidjson;

namespace Tmpl8 {

void Game::Init() {
  std::string filename = "assets/basic_ramp.gpmesh";
  std::ifstream ifs;
  ifs.open(filename);

  if (!ifs.is_open()) {
    cerr << "Could not open file" << endl;
    Game::Shutdown();
  }

  std::stringstream fileStream;
  fileStream << ifs.rdbuf();
  std::string gpmesh = fileStream.str();

  rapidjson::Document document;
  document.Parse(gpmesh.c_str());

  if (!document.IsObject()) {
    cerr << "Mesh " << filename << " is not a valid JSON." << endl;
    Game::Shutdown();
  }

  std::string shader = document["shader"].GetString();

  Value &vertsJSON = document["vertices"];
  assert(vertsJSON.IsArray() && vertsJSON.Size() > 0);

  std::vector<vec3> vertPositions;
  std::vector<vec3> vertNormals;
  vertPositions.reserve(vertsJSON.Size());
  vertNormals.reserve(vertsJSON.Size());

  for (int i = 0; i < vertsJSON.Size(); i++) {
    auto &vert = vertsJSON[i];
    assert(vert.IsArray() && vert.Size() == 16);

    vertPositions.emplace_back(
        vec3(vert[0].GetDouble(), vert[1].GetDouble(), vert[2].GetDouble()));
    vertNormals.emplace_back(
        vec3(vert[3].GetDouble(), vert[4].GetDouble(), vert[5].GetDouble()));
  }

  cout << vertPositions << endl;
  cout << vertNormals << endl;

  Value &indicesJSON = document["indices"];
  assert(indicesJSON.IsArray() && indicesJSON.Size() > 0);

  std::vector<unsigned int> indices;
  indices.reserve(indicesJSON.Size() * 3);

  for (int i = 0; i < indicesJSON.Size(); i++) {
    auto &index = indicesJSON[i];
    indices.emplace_back(index[0].GetUint());
    indices.emplace_back(index[1].GetUint());
    indices.emplace_back(index[2].GetUint());
  }

  cout << indices << endl;

  ifs.close();
}

void Game::Shutdown() {}

void Game::Tick(float deltaTime) {}

void Game::PhysicsTick(double t, double dt) {}
} // namespace Tmpl8
