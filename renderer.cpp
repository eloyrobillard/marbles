#include "renderer.h"
#include "camera.h"
#include "shader.h"

Renderer::Renderer(const unique_ptr<FollowCamera> &camera,
                   const shared_ptr<Surface> &screen)
    : mScreen(screen) {

  SetView(camera);
  SetProjection(screen);

  printf("application started.\n");
  SDL_Init(SDL_INIT_VIDEO);

  // Set OpenGL attributes
  // Use the core OpenGL profile
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  // Specify version 3.3
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  // Request a color buffer with 8-bits per RGBA channel
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  // Enable double buffering
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  // Force OpenGL to use hardware acceleration
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

#ifdef FULLSCREEN
  mWindow =
      SDL_CreateWindow(TemplateVersion, 100, 100, ScreenWidth, ScreenHeight,
                       SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
#else
  mWindow =
      SDL_CreateWindow(TemplateVersion, 100, 100, ScreenWidth, ScreenHeight,
                       SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
#endif

  mGlContext = SDL_GL_CreateContext(mWindow);

  GLenum status = glewInit();

  if (status != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(status));
  }

  printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

  mMeshShader = GetShader("shaders/basic.vert", "shaders/basic.frag");
  mColliderShader =
      GetShader("shaders/wireframe.vert", "shaders/wireframe.frag");
  mCollisionShader = GetShader("shaders/tint.vert", "shaders/tint.frag");
}

Renderer::~Renderer() {
  Shader::Unload(mMeshShader);
  Shader::Unload(mColliderShader);
  Shader::Unload(mCollisionShader);

  for (auto &tex : Texture::gAllTextures) {
    Texture::Unload(tex.second->textureID);
  }

  SDL_GL_DeleteContext(mGlContext);
  SDL_DestroyWindow(mWindow);
  SDL_Quit();
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

  SDL_GL_SwapWindow(mWindow);
}

void Renderer::SetView(const unique_ptr<FollowCamera> &camera) {
  mView =
      mat4::CreateLookAt(camera->mActualPosition, camera->mTarget, camera->mUp);
}

void Renderer::SetProjection(const shared_ptr<Surface> &screen) {
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

bool Renderer::createFBtexture() {
  glGenTextures(2, framebufferTexID);
  if (glGetError())
    return false;
  for (unsigned int texID : framebufferTexID) {
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ScreenWidth, ScreenHeight, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (glGetError())
      return false;
  }
  const int sizeMemory = 4 * ScreenWidth * ScreenHeight;
  glGenBuffers(2, fbPBO);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, fbPBO[0]);
  glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, sizeMemory, nullptr,
               GL_STREAM_DRAW_ARB);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, fbPBO[1]);
  glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, sizeMemory, nullptr,
               GL_STREAM_DRAW_ARB);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
  glBindTexture(GL_TEXTURE_2D, framebufferTexID[0]);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, fbPBO[0]);
  framedata = (unsigned const char *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB,
                                                 GL_WRITE_ONLY_ARB);
  if (!framedata)
    return false;
  memset((void *)framedata, 0, ScreenWidth * ScreenHeight * 4);
  return (glGetError() == 0);
}
