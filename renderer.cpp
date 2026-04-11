#include "renderer.h"
#include "camera.h"
#include "shader.h"

Renderer::Renderer(const unique_ptr<FollowCamera> &camera, Surface *screen)
    : mMeshShader(GetShader("shaders/basic.vert", "shaders/basic.frag")),
      mColliderShader(
          GetShader("shaders/wireframe.vert", "shaders/wireframe.frag")),
      mCollisionShader(GetShader("shaders/tint.vert", "shaders/tint.frag")) {

  SetView(camera);
  SetProjection(screen);
}

Renderer::~Renderer() {
  Shader::Unload(mMeshShader);
  Shader::Unload(mColliderShader);
  Shader::Unload(mCollisionShader);

  for (auto &tex : Texture::gAllTextures) {
    Texture::Unload(tex.second->textureID);
  }
}

void Renderer::Draw3D(float deltaTime, const unique_ptr<FollowCamera> &camera,
                      const vector<StaticEntity> &se,
                      const vector<DynamicEntity> &de) {
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

  while (!gTo_render_as_collided.empty()) {
    Mesh::setVerticesActive(gTo_render_as_collided.top());

    // Draw triangles
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

    gTo_render_as_collided.pop();
  }

  // Visualize triangle colliders as a wireframe
  Shader::setActive(mColliderShader);

  Shader::setMatrixUniform(mColliderShader, "uViewProj", viewProj);

  // Turn on wireframe mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Only show triangles currently tested against
  for (const auto &triangle : gCurrent_partition) {
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

  for (const auto &e : se) {
    Mesh::draw(mMeshShader, e.mesh, e.body);
  }

  for (const auto &e : de) {
    Mesh::draw(mMeshShader, e.mesh, e.body);
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

Shader::Shader Renderer::GetShader(const char *vert, const char *frag) {
  // Collider shader
  Shader::Shader shader = Shader::Load(vert, frag);

  if (!shader.isValid) {
    SDL_Log("Failed to load shader: %s", vert);
  }

  Shader::setActive(shader);

  return shader;
}
