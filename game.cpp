
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

void Game::Init() {
  GLenum status = glewInit();

  if (status != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(status));
  }

  printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  glEnable(GL_DEPTH_TEST);

  ramp = Mesh::load("assets/basic_ramp.gpmesh");

  if (!ramp.isValid) {
    cerr << "Error: failed to load mesh";
  }

  basicShader = Shader::load("shaders/basic.vert", "shaders/basic.frag");

  if (!basicShader.isValid) {
    cerr << "Error: failed to load shader";
  }

  Shader::setActive(basicShader);

  mat4 view = mat4::CreateLookAt(vec3::zero, vec3::right, vec3::forward);

  float fovy = 70.0f / 180.0f * PI;
  mat4 projection = mat4::CreatePerspectiveFOV(
      fovy, screen->GetWidth(), screen->GetHeight(), 25.0f, 10000.0f);

  Shader::setMatrixUniform(basicShader, "uViewProj", view * projection);
}

void Game::Shutdown() {}

void Game::Tick(float deltaTime) { Mesh::draw(basicShader, ramp); }

void Game::PhysicsTick(double t, double dt) {}
} // namespace Tmpl8
