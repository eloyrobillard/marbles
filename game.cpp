
#include "SDL_video.h"
#include "mesh.h"
#include "shader.h"
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
}

void Game::Shutdown() {}

void Game::Tick(float deltaTime) { Mesh::draw(basicShader, ramp); }

void Game::PhysicsTick(double t, double dt) {}
} // namespace Tmpl8
