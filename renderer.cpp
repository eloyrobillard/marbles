#include "renderer.h"
#include "camera.h"
#include "shader.h"

Renderer::Renderer(const unique_ptr<FollowCamera> &camera, Surface *screen)
    : mMeshShader(GetShader("shaders/basic.vert", "shaders/basic.frag")),
      mColliderShader(
          GetShader("shaders/wireframe.vert", "shaders/wireframe.frag")),
      mCollisionShader(GetShader("shaders/tint.vert", "shaders/tint.frag")) {

  GetMeshes({{"assets/twist.gpmesh", BodyType::Static},
             {"assets/sphere.gpmesh", BodyType::Dynamic}});

  SetView(camera);
  SetProjection(screen);
}

void Renderer::Draw3D(float deltaTime, const unique_ptr<FollowCamera> &camera) {
  SetView(camera);

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

  mat4 viewProj = mView * mProjection;

#ifdef _DEBUG
  // Visualize collisions
  Shader::setActive(mCollisionShader);

  Shader::setMatrixUniform(mCollisionShader, "uViewProj", viewProj);

  while (!to_render_as_collided.empty()) {
    Mesh::setVerticesActive(to_render_as_collided.top());

    // Draw triangles
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

    to_render_as_collided.pop();
  }

  // Visualize triangle colliders as a wireframe
  Shader::setActive(mColliderShader);

  Shader::setMatrixUniform(mColliderShader, "uViewProj", viewProj);

  // Turn on wireframe mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Only show triangles currently tested against
  for (const auto &triangle : current_partition) {
    Mesh::setVerticesActive(triangle.vertexArray);

    // Draw triangles
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
  }

  // Turn off wireframe mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif // _DEBUG

  Shader::setActive(mMeshShader);

  Shader::setMatrixUniform(mMeshShader, "uViewProj", viewProj);

  Shader::setLight(mMeshShader, mView);

  assert(mMeshes.size() == bodies.size());
  for (size_t i = 0; i < mMeshes.size(); i++) {
    Mesh::draw(mMeshShader, mMeshes[i], bodies[i]);
  }
}

void Renderer::SetView(const unique_ptr<FollowCamera> &camera) {
  mView =
      mat4::CreateLookAt(camera->mActualPosition, camera->mTarget, camera->mUp);
}

void Renderer::SetProjection(Surface *screen) {
  mProjection = mat4::CreatePerspectiveFOV(
      fovy, static_cast<float>(screen->GetWidth()),
      static_cast<float>(screen->GetHeight()), 1.0f, 10000.0f);
}

void Renderer::GetMeshes(const vector<pair<string, BodyType>> &meshList) {
  for (const auto &[meshName, btype] : meshList) {
    auto result = Mesh::load(meshName);

    if (result.has_value()) {
      auto [mesh, body] = result.value();

      if (btype == BodyType::Dynamic) {
        // Keep dynamic objects at the back of the queue
        mMeshes.emplace_back(mesh);
        bodies.emplace_back(body);
        dynamicColliders.emplace_back(body.position, body.scale.x);
      } else {
        auto triangles = Mesh::generateTriangleCollidersFromMesh(mesh, body);
        sp.populate(triangles);

        // Store static objects at the front of the queue
        mMeshes.emplace_front(mesh);
        bodies.emplace_front(body);

        staticColliders.emplace_back(triangles);
        num_static_bodies++;
      }
    }
  }
}

Shader::Shader Renderer::GetShader(const char *vert, const char *frag) {
  // Collider shader
  Shader::Shader shader = Shader::Load(vert, frag);

  if (!shader.isValid) {
    SDL_Log("Failed to load shader: %s", vert);
  }

  Shader::setActive(shader);

  return shader;
}
