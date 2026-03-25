
#include "mesh.h"
#include "shader.h"
#include "surface.h"
#include "template.h"
#include <cassert>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>

#include "game.h"
#include "glew.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace Tmpl8 {

Shader::Shader shader;
vector<Mesh::Mesh> meshes;

mat4 viewMat;
mat4 projMat;

void Game::Init() {
  vector<string> meshNames{"assets/basic_ramp.gpmesh", "assets/sphere.gpmesh"};
  for (const auto &meshName : meshNames) {
    Mesh::Mesh mesh = Mesh::load(meshName);

    if (mesh.isValid)
      meshes.emplace_back(Mesh::load(meshName));
  }

  shader = Shader::load("shaders/basic.vert", "shaders/basic.frag");

  if (!shader.isValid) {
    cerr << "Error: failed to load shader";
  }

  Shader::setActive(shader);

  viewMat = mat4::CreateLookAt(vec3::zero, vec3::forward, vec3::up);

  float fovy = 45.0f / 180.0f * PI;
  projMat = mat4::CreatePerspectiveFOV(fovy, screen->GetWidth(),
                                       screen->GetHeight(), 5.0f, 10000.0f);

  Shader::setMatrixUniform(shader, "uViewProj", viewMat * projMat);
}

void Game::Tick(float deltaTime) {
  // Set the clear color to light grey
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  // Clear the color buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw meshes
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  Shader::setActive(shader);

  // Update view-projection matrix
  Shader::setMatrixUniform(shader, "uViewProj", viewMat * projMat);

  for (auto &mesh : meshes) {
    Mesh::draw(shader, mesh);
  }
}

void Game::PhysicsTick(double t, double dt) {}

void Game::Shutdown() { Shader::unload(shader); }
} // namespace Tmpl8
