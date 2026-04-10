#include "game.h"
#include "camera.h"
#include "mesh.h"
#include "pch.h"
#include "physics.h"
#include "shader.h"
#include "surface.h"
#include "template.h"
#include "texture.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Tmpl8 {

unique_ptr<FollowCamera> camera;
// Used for spatial partitioning of static colliders
SPNode sp = SPNode(5.0f, 25.0f, 1, 16);
Shader::Shader meshShader;
Shader::Shader colliderShader;
std::deque<Mesh::Mesh> meshes;
std::deque<Body> bodies;
vector<SphereCollider> dynamicColliders;
vector<vector<TriangleCollider>> staticColliders;
int num_static_bodies = 0;

mat4 viewMat;
mat4 projMat;

enum class BodyType { Dynamic, Static };

void Game::Init() {
  camera =
      std::make_unique<FollowCamera>(vec3(0, 0, 10), vec3(7, 0, 0), vec3::up);

  vector<std::pair<string, BodyType>> meshNames{
      {"assets/twist.gpmesh", BodyType::Static},
      {"assets/sphere.gpmesh", BodyType::Dynamic}};

  for (const auto &[meshName, btype] : meshNames) {
    auto result = Mesh::load(meshName);

    if (result.has_value()) {
      auto [mesh, body] = result.value();

      if (btype == BodyType::Dynamic) {
        meshes.emplace_back(mesh);
        bodies.emplace_back(body);
        dynamicColliders.emplace_back(body.position, body.scale.x);
      } else {
        auto triangles = Mesh::generateTriangleCollidersFromMesh(mesh, body);
        sp.populate(triangles);

        meshes.emplace_front(mesh);
        bodies.emplace_front(body);
        staticColliders.emplace_back(triangles);
        num_static_bodies++;
      }
    }
  }

  meshShader = Shader::Load("shaders/basic.vert", "shaders/basic.frag");

  if (!meshShader.isValid) {
    SDL_Log("Failed to load mesh shader");
  }

  Shader::setActive(meshShader);

  colliderShader =
      Shader::Load("shaders/wireframe.vert", "shaders/wireframe.frag");

  if (!colliderShader.isValid) {
    SDL_Log("Failed to load collider shader");
  }

  Shader::setActive(colliderShader);

  viewMat =
      mat4::CreateLookAt(camera->mActualPosition, camera->mTarget, camera->mUp);

  float fovy = 30.0f / 180.0f * PI;
  projMat = mat4::CreatePerspectiveFOV(fovy, screen->GetWidth(),
                                       screen->GetHeight(), 1.0f, 10000.0f);
}

void Game::Tick(float deltaTime) {
  // Set the clear color to sky blue
  glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
  // Enable writing into the depth buffer
  glDepthMask(GL_TRUE);
  // Clear the color/depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw meshes
  // Enable depth buffering/disable alpha blend
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  Shader::setActive(meshShader);

  // Update view-projection matrix
  camera->update(bodies[num_static_bodies], deltaTime);
  viewMat =
      mat4::CreateLookAt(camera->mActualPosition, camera->mTarget, camera->mUp);

  Shader::setMatrixUniform(meshShader, "uViewProj", viewMat * projMat);

  Shader::setLight(meshShader, viewMat);

  assert(meshes.size() == bodies.size());
  for (size_t i = 0; i < meshes.size(); i++) {
    Mesh::draw(meshShader, meshes[i], bodies[i]);
  }

#ifdef _DEBUG
  Shader::setActive(colliderShader);

  Shader::setMatrixUniform(colliderShader, "uViewProj", viewMat * projMat);

  // Turn on wireframe mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  for (const auto &v : staticColliders) {
    for (const auto &triangle : v) {
      Mesh::setVerticesActive(triangle.vertexArray);

      // Draw triangles
      glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
    }
  }

  // Turn off wireframe mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif // _DEBUG
}

void Game::PhysicsTick(float time, float dt) {
  for (int i = num_static_bodies; i < bodies.size(); i++) {
    Physics::Update(bodies[i], time, dt, dynamicColliders,
                    dynamicColliders[i - num_static_bodies], sp);
  }
}

void Game::Shutdown() {
  Shader::Unload(meshShader);

  for (auto &tex : Texture::gAllTextures) {
    Texture::Unload(tex.second->textureID);
  }
}
} // namespace Tmpl8
