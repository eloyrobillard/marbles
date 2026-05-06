#include "renderer.h"
#include "camera.h"
#include "shader.h"
#include "template.h"

TTF_Font *TTF_GetFont(const char *fontName, float ptsize,
                      TTF_FontStyleFlags fontStyleFlags) {
  TTF_Font *font = TTF_OpenFont(fontName, ptsize);

  if (!font) {
    SDL_Log("TTF_OpenFont: %s\n", SDL_GetError());
  }

  TTF_SetFontStyle(font, fontStyleFlags);

  return font;
}

// SOURCE: https://lackeyccg.com/glfont.c
void SDL_GL_Enter2DMode() {
  glDisable(GL_DEPTH_TEST);

  /* This allows alpha blending of 2D textures with the scene */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SDL_GL_Leave2DMode() {
  glEnable(GL_DEPTH_TEST);

  /* This allows alpha blending of 2D textures with the scene */
  glDisable(GL_BLEND);
}

// SOURCE: https://lackeyccg.com/glfont.c
GLuint SDL_GL_LoadTexture(SDL_Surface *surface, shared_ptr<Surface> &screen,
                          int dst_x, int dst_y, SDL_FlipMode flip_mode) {
  int w = screen->GetWidth();
  int h = screen->GetHeight();

  SDL_Surface *image = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_ARGB32);

  if (image == nullptr) {
    return 0;
  }

  /* Save the alpha blending attributes */
  Uint8 saved_alpha;
  SDL_BlendMode saved_mode;
  SDL_GetSurfaceAlphaMod(surface, &saved_alpha);
  SDL_SetSurfaceAlphaMod(surface, 0xFF);
  SDL_GetSurfaceBlendMode(surface, &saved_mode);
  SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

  /* Copy the surface into the GL texture image */
  SDL_Rect src;
  src.x = 0;
  src.y = 0;
  src.w = surface->w;
  src.h = surface->h;

  SDL_Rect dst;
  dst.x = dst_x;
  dst.y = dst_y;
  dst.w = surface->w;
  dst.h = surface->h;

  SDL_BlitSurface(surface, &src, image, &dst);
  SDL_FlipSurface(image, flip_mode);

  /* Restore the alpha blending attributes */
  SDL_SetSurfaceAlphaMod(surface, saved_alpha);
  SDL_SetSurfaceBlendMode(surface, saved_mode);

  /* Create an OpenGL texture for the image */
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               image->pixels);

  SDL_DestroySurface(image); /* No longer needed */

  return texture;
}

Renderer::Renderer(const shared_ptr<Surface> &screen) : mScreen(screen) {
  setProjection(screen);

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
  mWindow = SDL_CreateWindow("Marbles", ScreenWidth, ScreenHeight,
                             SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
#else
  mWindow =
      SDL_CreateWindow("Marbles", ScreenWidth, ScreenHeight, SDL_WINDOW_OPENGL);
#endif

  mGlContext = SDL_GL_CreateContext(mWindow);

  GLenum status = glewInit();

  if (status != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(status));
  }

  printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

  mTextShader = GetShader("shaders/text.vert", "shaders/text.frag");
  mMeshShader = GetShader("shaders/basic.vert", "shaders/basic.frag");
  mColliderShader =
      GetShader("shaders/wireframe.vert", "shaders/wireframe.frag");
  mCollisionShader = GetShader("shaders/tint.vert", "shaders/tint.frag");
  mPostShader = GetShader("shaders/post.vert", "shaders/post.frag");
  mSkyboxShader = GetShader("shaders/skybox.vert", "shaders/skybox.frag");

  setupSkybox();

  // Setup AA and depth framebuffers
  setupFramebuffers();

  // Setup HUD
  if (!TTF_Init()) {
    SDL_Log("TTF_Init error: %s\n", SDL_GetError());
  }

  TTF_Font *font =
      TTF_GetFont("assets/fonts/NotoSansCJKjp-VF.ttf", 30, TTF_STYLE_BOLD);

  // Length can be zero for null-terminated text
  SDL_Surface *commandsSurface = TTF_RenderText_Blended_Wrapped(
      font, "Left/Right arrows to turn\nSpace to restart", 0,
      {255, 255, 255, 255}, 0);

  GLuint hudTexture =
      SDL_GL_LoadTexture(commandsSurface, mScreen, 5, 5, SDL_FLIP_VERTICAL);

  SDL_DestroySurface(commandsSurface);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  pushHUDTexture(hudTexture);

  TTF_SetFontSize(font, 60);
  SDL_Surface *loadingSurface = TTF_RenderText_Blended_Wrapped(
      font, "...Loading", 0, {255, 255, 255, 255}, 0);

  TTF_SetFontSize(font, 200);
  SDL_Surface *victorySurface = TTF_RenderText_Blended_Wrapped(
      font, "You win!", 0, {255, 255, 255, 255}, 0);

  TTF_CloseFont(font);

  mLoadingTexture = SDL_GL_LoadTexture(
      loadingSurface, mScreen, mScreen->GetWidth() - loadingSurface->w - 10,
      mScreen->GetHeight() - loadingSurface->h - 10, SDL_FLIP_VERTICAL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  mVictoryTexture = SDL_GL_LoadTexture(
      victorySurface, mScreen, (mScreen->GetWidth() - victorySurface->w) >> 1,
      (mScreen->GetHeight() - victorySurface->h) >> 1, SDL_FLIP_VERTICAL);

  SDL_DestroySurface(loadingSurface);
  SDL_DestroySurface(victorySurface);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Draw loading screen
  SDL_GL_Enter2DMode();
  float textColor[3] = {1.0f, 1.0f, 1.0f};
  drawToHUD(hudVAO, mLoadingTexture, textColor);
  SDL_GL_Leave2DMode();

  SDL_GL_SwapWindow(mWindow);
}

void Renderer::drawToHUD(GLuint VAO, GLuint texture, const float textColor[3]) {
  mTextShader.SetActive();
  mTextShader.SetVec3Uniform("textColor", textColor);
  glBindVertexArray(VAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  // use the now resolved color attachment as the quad's texture
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

bool Renderer::setupSkybox() {
  vector<std::string> faces{
      "assets/skybox/right.jpg", "assets/skybox/left.jpg",
      "assets/skybox/top.jpg",   "assets/skybox/bottom.jpg",
      "assets/skybox/front.jpg", "assets/skybox/back.jpg"};

  skyboxTexture = Texture::LoadCubemap(faces);

  return setupSkyboxVAO();
}

void Renderer::SetCamera(const shared_ptr<FollowCamera> &camera) {
  mCamera = camera;
  setView(camera);
}

Renderer::~Renderer() {
  mMeshShader.Unload();
  mColliderShader.Unload();
  mCollisionShader.Unload();
  mTextShader.Unload();
  mPostShader.Unload();
  mSkyboxShader.Unload();

  for (const auto &[_, tex] : gAllTextures) {
    tex->Unload();
  }

  TTF_Quit();
  SDL_GL_DestroyContext(mGlContext);
  SDL_DestroyWindow(mWindow);
  SDL_Quit();
}

bool Renderer::setupSkyboxVAO() {
  // clang-format off
  float skyboxVertices[] = {
      // positions          
      -1.0f,  1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

      -1.0f,  1.0f, -1.0f,
       1.0f,  1.0f, -1.0f,
       1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
       1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
       1.0f, -1.0f,  1.0f
  };
  // clang-format on

  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  return (glGetError() == 0);
}

void Renderer::setupScreenQuadVAO(GLuint &VAO, GLuint &VBO) {
  // vertex attributes for a quad that fills the entire screen in Normalized
  // Device Coordinates. (positions, texCoords)
  float quadVertices[] = {-1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
                          0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

                          -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
                          1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f};

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
}

// SOURCE: https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing
bool Renderer::setupFramebuffers() {
  setupScreenQuadVAO(hudVAO, hudVBO);
  setupScreenQuadVAO(quadVAO, quadVBO);

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
    cout
        << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete !\n ";
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // post-processing shader config
  mPostShader.SetActive();
  mPostShader.SetIntUniform("screenTexture", 0);

  return (glGetError() == 0);
}

void Renderer::drawSkybox() {
  // draw skybox behind scene
  glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when
                          // values are equal to depth buffer's content
  mSkyboxShader.SetActive();
  mSkyboxShader.SetMatrixUniform(
      "view", mat4::CreateLookAtSkybox(mCamera->mActualPosition,
                                       mCamera->mTarget, mCamera->mUp));
  mSkyboxShader.SetMatrixUniform("projection", mProjection);

  // skybox cube
  glBindVertexArray(skyboxVAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glDepthFunc(GL_LESS); // set depth function back to default
}

void Renderer::drawDebug(const mat4 &viewProj) {
  // Visualize collisions
  mCollisionShader.SetActive();

  mCollisionShader.SetMatrixUniform("uViewProj", viewProj);

  while (!gTo_render_as_collided.empty()) {
    Shader::SetVerticesActive(gTo_render_as_collided.top());

    // Draw triangles
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

    gTo_render_as_collided.pop();
  }

  // Visualize triangle colliders as a wireframe
  mColliderShader.SetActive();

  mColliderShader.SetMatrixUniform("uViewProj", viewProj);

  // Turn on wireframe mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Only show triangles currently tested against
  for (const auto &triangle : gCurrent_partition) {
    Shader::SetVerticesActive(triangle.vertexArray);

    // Draw triangles
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
  }

  // Turn off wireframe mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::drawEntities(const shared_ptr<const Entities> &entities,
                            const mat4 &viewProj) {
  mMeshShader.SetActive();

  mMeshShader.SetMatrixUniform("uViewProj", viewProj);

  mMeshShader.SetLight(mView);

  entities->DrawStaticEntities(mMeshShader);
  entities->DrawDynamicEntities(mMeshShader);
}

void Renderer::Draw3D(float deltaTime,
                      const shared_ptr<const Entities> &entities) {
  setView(mCamera);

  // Clear the color/depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 1. draw scene as normal in multisampled buffers
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  // Set the clear color
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  mat4 viewProj = mView * mProjection;

#ifdef _DEBUG
  drawDebug(viewProj);
#endif // _DEBUG

  drawEntities(entities, viewProj);

  drawSkybox();

  // finally, draw HUD elements
  SDL_GL_Enter2DMode();
  {
    mTextShader.SetActive();
    const float textColor[3] = {1.0f, 1.0f, 1.0f};
    mTextShader.SetVec3Uniform("textColor", textColor);
    glBindVertexArray(hudVAO);
    glActiveTexture(GL_TEXTURE0);

    if (!mShowVictoryMessage) {
      for (auto text : hudTextures) {
        glBindTexture(GL_TEXTURE_2D, text);
        glDrawArrays(GL_TRIANGLES, 0, 6);
      }
    } else {
      glBindTexture(GL_TEXTURE_2D, mVictoryTexture);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);

    // 2. now blit multisampled buffer(s) to normal colorbuffer of intermediate
    // FBO. Image is stored in screenTexture
    blitFramebuffer(framebuffer, intermediateFBO, mScreen->GetWidth(),
                    mScreen->GetHeight(), mScreen->GetWidth(),
                    mScreen->GetHeight());

    // 3. now render quad with scene's visuals as its texture image
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // draw the result of every other draw
    drawScreenQuad(mPostShader, quadVAO, screenTexture);
  }
  SDL_GL_Leave2DMode();

  SDL_GL_SwapWindow(mWindow);
}

void Renderer::blitFramebuffer(GLuint readFB, GLuint drawFB, int readW,
                               int readH, int drawW, int drawH) {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, readFB);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFB);
  glBlitFramebuffer(0, 0, readW, readH, 0, 0, drawW, drawH, GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);
}

void Renderer::drawScreenQuad(Shader &shader, GLuint VAO, GLuint texture) {
  shader.SetActive();
  glBindVertexArray(VAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  // use the now resolved color attachment as the quad's texture
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::setView(const shared_ptr<FollowCamera> &camera) {
  mView =
      mat4::CreateLookAt(camera->mActualPosition, camera->mTarget, camera->mUp);
}

void Renderer::setProjection(const shared_ptr<Surface> &screen) {
  mProjection = mat4::CreatePerspectiveFOV(
      fovy, static_cast<float>(screen->GetWidth()),
      static_cast<float>(screen->GetHeight()), 1.0f, 10000.0f);
}

Shader Renderer::GetShader(const char *vert, const char *frag) {
  // Collider shader
  optional<Shader> maybe_shader = Shader::Load(vert, frag);

  if (!maybe_shader.has_value()) {
    SDL_Log("Failed to load shader: %s", vert);
  }

  maybe_shader.value().SetActive();

  return maybe_shader.value_or(Shader());
}
