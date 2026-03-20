
#include "SDL_video.h"
#include "mesh.h"
#include "shader.h"
#include "surface.h"
#include "template.h"
#include <cassert>
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>

#include "game.h"
#include "glew.h"

using std::cerr;
using std::cout;
using std::endl;

namespace Tmpl8 {

Shader::Shader basicShader;
Mesh::Mesh ramp;

mat4 viewMat;
mat4 projMat;

void Game::Init() {
  ramp = Mesh::load("assets/basic_ramp.gpmesh");

  if (!ramp.isValid) {
    cerr << "Error: failed to load mesh";
  }

  basicShader = Shader::load("shaders/basic.vert", "shaders/basic.frag");

  if (!basicShader.isValid) {
    cerr << "Error: failed to load shader";
  }

  Shader::setActive(basicShader);

  viewMat = mat4::CreateLookAt(vec3::zero, vec3::forward, vec3::up);

  float fovy = 70.0f / 180.0f * PI;
  projMat = mat4::CreatePerspectiveFOV(fovy, screen->GetWidth(),
                                       screen->GetHeight(), 25.0f, 10000.0f);

  Shader::setMatrixUniform(basicShader, "uViewProj", viewMat * projMat);
}

void Game::Tick(float deltaTime) {
  // Set the clear color to light grey
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  // Clear the color buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw meshes
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  Shader::setActive(basicShader);

  // Update view-projection matrix
  Shader::setMatrixUniform(basicShader, "uViewProj", viewMat * projMat);

  Mesh::draw(basicShader, ramp);
}

void Game::PhysicsTick(double t, double dt) {}

void Game::Shutdown() { Shader::unload(basicShader); }
} // namespace Tmpl8
