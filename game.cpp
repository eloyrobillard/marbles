
#include "game.h"
#include "mesh.h"
#include "pch.h"
#include "physics.h"
#include "shader.h"
#include "surface.h"
#include "template.h"
#include "texture.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace Tmpl8 {

Shader::Shader meshShader;
std::deque<Mesh::Mesh> meshes;
std::deque<Body> bodies;
vector<SphereCollider> dynamicColliders;
vector<vector<TriangleCollider>> staticColliders;
int num_static_bodies = 0;

mat4 viewMat;
mat4 projMat;

enum class BodyType { Dynamic, Static };
// enum class ColliderType { Sphere, Complex };

void Game::Init() {
  vector<std::pair<string, BodyType>> meshNames{
      {"assets/basic_ramp.gpmesh", BodyType::Static},
      {"assets/sphere.gpmesh", BodyType::Dynamic}};

  for (const auto &[meshName, btype] : meshNames) {
    auto [mesh, body] = Mesh::load(meshName);

    if (mesh.isValid) {
      if (btype == BodyType::Dynamic) {
        meshes.emplace_back(mesh);
        bodies.emplace_back(body);
        dynamicColliders.emplace_back(&body.position, 3.0f);
      } else {
        meshes.emplace_front(mesh);
        bodies.emplace_front(body);
        staticColliders.emplace_back(
            Mesh::generateTriangleCollidersFromMesh(mesh, body));
        num_static_bodies++;
      }
    }
  }

  meshShader = Shader::load("shaders/basic.vert", "shaders/basic.frag");

  if (!meshShader.isValid) {
    SDL_Log("Failed to load mesh shader");
  }

  Shader::setActive(meshShader);

  viewMat = mat4::CreateLookAt(vec3::zero, vec3::forward, vec3::up);

  float fovy = 45.0f / 180.0f * PI;
  projMat = mat4::CreatePerspectiveFOV(fovy, screen->GetWidth(),
                                       screen->GetHeight(), 5.0f, 10000.0f);

  Shader::setMatrixUniform(meshShader, "uViewProj", viewMat * projMat);
}

void Game::Tick(float deltaTime) {
  // Set the clear color to light grey
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  // Clear the color buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw meshes
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  Shader::setActive(meshShader);

  // Update view-projection matrix
  Shader::setMatrixUniform(meshShader, "uViewProj", viewMat * projMat);

  assert(meshes.size() == bodies.size());
  for (size_t i = 0; i < meshes.size(); i++) {
    Mesh::draw(meshShader, meshes[i], bodies[i]);
  }
}

void Game::PhysicsTick(float time, float dt) {
  for (int i = num_static_bodies; i < bodies.size(); i++) {
    Physics::Update(bodies[i], time, dt, staticColliders, dynamicColliders,
                    dynamicColliders[i - num_static_bodies]);
  }
}

void Game::Shutdown() {
  Shader::unload(meshShader);

  for (auto &tex : Texture::gAllTextures) {
    Texture::Unload(tex.second->textureID);
  }
}
} // namespace Tmpl8
