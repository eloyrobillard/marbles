#include "game.h"
#include "camera.h"
#include "mesh.h"
#include "pch.h"
#include "physics.h"
#include "renderer.h"
#include "shader.h"
#include "surface.h"
#include "template.h"
#include "texture.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Tmpl8 {

unique_ptr<FollowCamera> camera;
unique_ptr<Renderer> renderer;

Shader::Shader meshShader;
Shader::Shader colliderShader;
Shader::Shader collisionShader;

void Game::Init() {
  camera =
      std::make_unique<FollowCamera>(vec3(0, 0, 3), vec3(7, 0, 0), vec3::up);

  renderer = std::make_unique<Renderer>(Renderer(camera, screen));
}

void Game::Tick(float deltaTime) {
  camera->update(bodies[num_static_bodies], deltaTime);

  renderer->Draw3D(deltaTime, camera);
}

void Game::PhysicsTick(float time, float dt) {
  for (int i = num_static_bodies; i < bodies.size(); i++) {
    Physics::Update(bodies[i], time, dt,
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
