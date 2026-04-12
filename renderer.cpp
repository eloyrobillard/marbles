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
  mPostShader = GetShader("shaders/post.vert", "shaders/post.frag");

  setupFramebuffers();
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

// SOURCE: https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing
bool Renderer::setupFramebuffers() {
  // vertex attributes for a quad that fills the entire screen in Normalized
  // Device Coordinates. (positions, texCoords)
  float quadVertices[] = {-1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
                          0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

                          -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
                          1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f};

  // setup screen VAO
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));

  // configure MSAA framebuffer
  // --------------------------
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  // create a multisampled color attachment texture
  unsigned int textureColorBufferMultiSampled;
  glGenTextures(1, &textureColorBufferMultiSampled);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB,
                          mScreen->GetWidth(), mScreen->GetHeight(), GL_TRUE);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D_MULTISAMPLE,
                         textureColorBufferMultiSampled, 0);

  // create a (also multisampled) renderbuffer object for depth and stencil
  // attachments
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8,
                                   mScreen->GetWidth(), mScreen->GetHeight());
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;

  // configure second post-processing framebuffer
  glGenFramebuffers(1, &intermediateFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

  // create a color attachment texture
  glGenTextures(1, &screenTexture);
  glBindTexture(GL_TEXTURE_2D, screenTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mScreen->GetWidth(),
               mScreen->GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         screenTexture, 0); // we only need a color buffer

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!\n";
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // post-processing shader config
  Shader::setActive(mPostShader);
  Shader::setIntUniform(mPostShader, "screenTexture", 0);

  return (glGetError() == 0);
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

  // 1. draw scene as normal in multisampled buffers
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

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

  // 2. now blit multisampled buffer(s) to normal colorbuffer of intermediate
  // FBO. Image is stored in screenTexture
  glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
  glBlitFramebuffer(0, 0, mScreen->GetWidth(), mScreen->GetHeight(), 0, 0,
                    mScreen->GetWidth(), mScreen->GetHeight(),
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);

  // 3. now render quad with scene's visuals as its texture image
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  // draw Screen quad
  Shader::setActive(mPostShader);
  glBindVertexArray(quadVAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, screenTexture);
  // use the now resolved color attachment as the quad's texture
  glDrawArrays(GL_TRIANGLES, 0, 6);

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
