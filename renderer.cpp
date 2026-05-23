#include "renderer.h"
#include "camera.h"
#include "shader.h"

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
GLuint Renderer::LoadGLTexture(SDL_Surface *surface, int dst_x, int dst_y,
                               SDL_FlipMode flip_mode) const {
  int w = mScreenWidth;
  int h = mScreenHeight;

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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               image->pixels);

  SDL_DestroySurface(image); /* No longer needed */

  return texture;
}

Renderer::Renderer(bool goFullscreen, int screenWidth, int screenHeight) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

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

  mIsFullscreen = goFullscreen;
  if (goFullscreen) {
    mWindow = SDL_CreateWindow("Marbles", screenWidth, screenHeight,
                               SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
  } else {
    mWindow = SDL_CreateWindow("Marbles", screenWidth, screenHeight,
                               SDL_WINDOW_OPENGL);
  }

  // Save final screen width/height
  SDL_GetWindowSize(mWindow, &mScreenWidth, &mScreenHeight);

  mAspectRatio =
      static_cast<float>(mScreenWidth) / static_cast<float>(mScreenHeight);

  setProjection();

  mGlContext = SDL_GL_CreateContext(mWindow);

  GLenum status = glewInit();

  if (status != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(status));
  }

  printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

  mDepthMapShader = GetShader("shaders/depthMap.vert", "shaders/depthMap.frag");
  mDebugShadowMapShader =
      GetShader("shaders/debugShadowMap.vert", "shaders/debugShadowMap.frag");
  mShadowMapShader =
      GetShader("shaders/shadowMap.vert", "shaders/shadowMap.frag");
  mContourShader = GetShader("shaders/contour.vert", "shaders/contour.frag");
  mDrawStaticShader =
      GetShader("shaders/drawStatic.vert", "shaders/drawStatic.frag");

  mTextShader = GetShader("shaders/text.vert", "shaders/text.frag");

  mMeshShader = GetShader("shaders/basic.vert", "shaders/basic.frag");
  mColliderShader =
      GetShader("shaders/wireframe.vert", "shaders/wireframe.frag");
  mCollisionShader = GetShader("shaders/tint.vert", "shaders/tint.frag");
  mPostShader = GetShader("shaders/post.vert", "shaders/post.frag");
  mGaussianBlurShader =
      GetShader("shaders/gaussianBlur.vert", "shaders/gaussianBlur.frag");
  mApplyBloomShader =
      GetShader("shaders/applyBloom.vert", "shaders/applyBloom.frag");

  mDrawStaticShader.SetActive();
  mDrawStaticShader.SetIntUniform("meshTex", 0);
  mDrawStaticShader.SetIntUniform("depthTex", 1);

  mTextShader.SetActive();
  mTextShader.SetIntUniform("text", 0);
  mTextShader.SetIntUniform("screen", 1);

  mMeshShader.SetActive();
  mMeshShader.SetIntUniform("uSamplingTexture", 0);

  mGaussianBlurShader.SetActive();
  mGaussianBlurShader.SetIntUniform("brightTexture", 0);

  mApplyBloomShader.SetActive();
  mApplyBloomShader.SetIntUniform("normalTex", 0);
  mApplyBloomShader.SetIntUniform("bloomTex", 1);

  // Setup AA and depth framebuffers
  setupFramebuffers();

  // Setup HUD
  if (!TTF_Init()) {
    SDL_Log("TTF_Init error: %s\n", SDL_GetError());
  }

  mFont = TTF_GetFont("assets/fonts/FiraCodeNerdFontMono-Regular.ttf", 30,
                      TTF_STYLE_NORMAL);

  // Length can be zero for null-terminated text
  SDL_Surface *commandsSurface = TTF_RenderText_Blended_Wrapped(
      mFont, "Left/Right arrows to turn\nSpace to restart", 0,
      {255, 255, 255, 255}, 0);

  GLuint hudTexture = LoadGLTexture(commandsSurface, 5, 5, SDL_FLIP_VERTICAL);

  SDL_DestroySurface(commandsSurface);

  pushHUDTexture(hudTexture);

  TTF_SetFontSize(mFont, 60);
  SDL_Surface *loadingSurface = TTF_RenderText_Blended_Wrapped(
      mFont, "...Loading", 0, {255, 255, 255, 255}, 0);

  TTF_SetFontSize(mFont, 200);
  SDL_Surface *victorySurface = TTF_RenderText_Blended_Wrapped(
      mFont, "You win!", 0, {255, 255, 255, 255}, 0);

  mLoadingTexture =
      LoadGLTexture(loadingSurface, mScreenWidth - loadingSurface->w - 10,
                    mScreenHeight - loadingSurface->h - 10, SDL_FLIP_VERTICAL);

  mVictoryTexture = LoadGLTexture(
      victorySurface, (mScreenWidth - victorySurface->w) >> 1,
      (mScreenHeight - victorySurface->h) >> 1, SDL_FLIP_VERTICAL);

  SDL_DestroySurface(loadingSurface);
  SDL_DestroySurface(victorySurface);

  // Draw loading screen
  SDL_GL_Enter2DMode();
  float textColor[3] = {1.0f, 1.0f, 1.0f};
  drawToHUD(quadVAO, mLoadingTexture, mScreenTexture, textColor);
  SDL_GL_Leave2DMode();

  // Font size for marble coordinates HUD
  TTF_SetFontSize(mFont, 30);

  SDL_GL_SwapWindow(mWindow);
}

void Renderer::drawEntity(const Shader &shader, const Entity &entity) {
  const auto &[worldTransform, maybe_tex, vertexArray, numIndices] =
      entity.GetDrawData();
  shader.SetMatrixUniform("uWorldTransform", worldTransform);

  if (maybe_tex.has_value()) {
    glActiveTexture(GL_TEXTURE0);
    maybe_tex.value()->SetActive();
  }

  Shader::SetVerticesActive(vertexArray);

  // Draw triangles
  glDrawElements(GL_TRIANGLES, static_cast<int>(numIndices), GL_UNSIGNED_INT,
                 nullptr);

  GLenum err_code = glGetError();
  while (GL_NO_ERROR != err_code) {
    printf("OpenGL Error @ %s: %i", "mesh draw", err_code);
    err_code = glGetError();
  }
}

void Renderer::drawToHUD(GLuint VAO, GLuint textTexture, GLuint screenTexture,
                         const float textColor[3]) {
  mTextShader.SetActive();
  mTextShader.SetVec3Uniform("textColor", textColor);
  glBindVertexArray(VAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textTexture);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, screenTexture);
  // use the now resolved color attachment as the quad's texture
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
  mGaussianBlurShader.Unload();
  mApplyBloomShader.Unload();

  for (const auto &[_, tex] : gAllTextures) {
    tex->Unload();
  }

  TTF_CloseFont(mFont);
  TTF_Quit();
  SDL_GL_DestroyContext(mGlContext);
  SDL_DestroyWindow(mWindow);
  SDL_Quit();
}

void Renderer::setupQuadVAO(GLuint &VAO, GLuint &VBO) {
  // vertex attributes for a quad that fills the entire screen in Normalized
  // Device Coordinates.
  // clang-format off
  float quadVertices[] = {
      // positions        // texture Coords
      -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
       1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
       1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
  };
  // clang-format on

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
}

GLuint Renderer::createColorAttachmentTexture(int width, int height,
                                              int colorFormat,
                                              int colorAttachmentNumber) {
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachmentNumber, GL_TEXTURE_2D,
                         texture, 0);

  return texture;
}

void Renderer::configureMultiSampledAntiAliasing() {
  glGenFramebuffers(1, &mMSAAFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, mMSAAFBO);
  // create a multisampled color attachment texture
  unsigned int textureColorBufferMultiSampled;
  glGenTextures(1, &textureColorBufferMultiSampled);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, mScreenWidth,
                          mScreenHeight, GL_TRUE);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D_MULTISAMPLE,
                         textureColorBufferMultiSampled, 0);

  // create a (also multisampled) renderbuffer object for depth and stencil
  // attachments
  glGenRenderbuffers(1, &mMSAARenderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, mMSAARenderBuffer);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8,
                                   mScreenWidth, mScreenHeight);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, mMSAARenderBuffer);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
}

bool Renderer::setupFramebuffers() {
  setupQuadVAO(quadVAO, quadVBO);

  // depth map
  glGenTextures(1, &mDepthMapTexture);
  glBindTexture(GL_TEXTURE_2D, mDepthMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mScreenWidth,
               mScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  glGenFramebuffers(1, &mDepthMapFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, mDepthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         mDepthMapTexture, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // framebuffer to be reused while rendering entity outlines
  glGenFramebuffers(1, &mContourFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, mContourFBO);
  mContourColorBuffer =
      createColorAttachmentTexture(mScreenWidth, mScreenHeight, GL_RGB);

  // configure shadow map framebuffer
  glGenFramebuffers(1, &mShadowMapFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, mShadowMapFBO);

  // create texture for shadow map
  glGenTextures(1, &mShadowMapTexture);
  glBindTexture(GL_TEXTURE_2D, mShadowMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mScreenWidth,
               mScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         mShadowMapTexture, 0);

  configureMultiSampledAntiAliasing();

  // configure second post-processing framebuffer
  glGenFramebuffers(1, &mIntermediateFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, mIntermediateFBO);

  // create a color attachment texture for second framebuffer
  mScreenTexture =
      createColorAttachmentTexture(mScreenWidth, mScreenHeight, GL_RGB16F);
  // create a second color attachment, this time for the bloom effect
  // NOTE: using GL_RGB16F format to prevent clamping of values written to
  // buffer. This is to allow HDR.
  mBrightnessTexture = createColorAttachmentTexture(
      mScreenWidth, mScreenHeight, GL_RGB16F, GL_COLOR_ATTACHMENT1);

  // Tell OpenGL to render to both of above buffers
  unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete !\n";
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Used for Gaussian blur (bloom effect)
  glGenFramebuffers(2, mPingpongFBO);
  // Textures -> no need for depth buffer
  glGenTextures(2, mPingpongColorBuffer);
  for (uint i = 0; i < 2; i++) {
    glBindFramebuffer(GL_FRAMEBUFFER, mPingpongFBO[i]);
    glBindTexture(GL_TEXTURE_2D, mPingpongColorBuffer[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mScreenWidth, mScreenHeight, 0,
                 GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           mPingpongColorBuffer[i], 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      cout << "ERROR::FRAMEBUFFER:: Ping-pong framebuffer is not complete !\n";
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return (glGetError() == 0);
}

void Renderer::drawDebug(const mat4 &viewProj) {
  // override previous triangle draw no matter what
  glDisable(GL_DEPTH_TEST);

  // Visualize collisions
  mCollisionShader.SetActive();

  mCollisionShader.SetMatrixUniform("uViewProj", viewProj);

  while (!gTo_render_as_collided.empty()) {
    Shader::SetVerticesActive(gTo_render_as_collided.top());

    // Draw triangles
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

    gTo_render_as_collided.pop();
  }

  glEnable(GL_DEPTH_TEST);

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

void Renderer::drawSceneWithShader(const Shader &shader,
                                   const shared_ptr<const Entities> &entities,
                                   const mat4 &viewProj) {
  shader.SetActive();
  shader.SetMatrixUniform("uViewProj", viewProj);

  for (const auto &e : entities->GetStaticEntities()) {
    drawEntity(shader, e);
  }

  for (const auto &e : entities->GetDynamicEntities()) {
    drawEntity(shader, e);
  }
}

void Renderer::drawScene(const shared_ptr<const Entities> &entities,
                         const mat4 &viewProj) {
  glBindFramebuffer(GL_FRAMEBUFFER, mDepthMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  mDepthMapShader.SetActive();
  mDepthMapShader.SetMatrixUniform("uViewProj", viewProj);

  for (const auto &e : entities->GetStaticEntities()) {
    drawEntity(mDepthMapShader, e);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, mContourFBO);
  mContourShader.SetActive();
  drawQuad(mContourShader, quadVAO, mDepthMapTexture);

  glBindFramebuffer(GL_FRAMEBUFFER, mMSAAFBO);
  mDrawStaticShader.SetActive();
  mDrawStaticShader.SetMatrixUniform("uViewProj", viewProj);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, mContourColorBuffer);

  for (const auto &e : entities->GetStaticEntities()) {
    drawEntity(mDrawStaticShader, e);
  }

#ifdef _DEBUG
  drawDebug(viewProj);
#endif // _DEBUG

  mMeshShader.SetActive();
  mMeshShader.SetMatrixUniform("uViewProj", viewProj);

  for (const auto &e : entities->GetDynamicEntities()) {
    drawEntity(mMeshShader, e);
  }
}

void Renderer::Draw3D(float deltaTime,
                      const shared_ptr<const Entities> &entities) {
  setView(mCamera);

  // 1. draw scene as normal in multisampled buffers
  glBindFramebuffer(GL_FRAMEBUFFER, mMSAAFBO);
  // Set the clear color
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);

  mat4 viewProj = mView * mProjection;

  // Setup light for shadow mapping
  float near = 1.0f, far = 100.0f;
  const vec3 lightPos{40.0f, -3.0f, 3.0f};
  const vec3 lightTarget{40.0f, 0.0f, -1.0f};
  const mat4 lightView = mat4::CreateLookAt(lightPos, lightTarget, vec3::up);
  const mat4 lightProj = mat4::CreateOrtho(100 * mAspectRatio, 100, near, far);

  glBindFramebuffer(GL_FRAMEBUFFER, mShadowMapFBO);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawSceneWithShader(mShadowMapShader, entities, lightView * lightProj);

  drawScene(entities, viewProj);

  // finally, draw HUD elements
  SDL_GL_Enter2DMode();
  {
    // 2. now blit multisampled buffer(s) to normal colorbuffer of intermediate
    // FBO. Image is stored in screenTexture
    blitFramebuffer(mMSAAFBO, mIntermediateFBO, mScreenWidth, mScreenHeight,
                    mScreenWidth, mScreenHeight);

    mGaussianBlurShader.SetActive();
    bool horizontal = true;
    glBindFramebuffer(GL_FRAMEBUFFER, mPingpongFBO[horizontal]);
    drawQuad(mGaussianBlurShader, quadVAO, mBrightnessTexture);
    for (int i = 0; i < 10; i++) {
      mGaussianBlurShader.SetBoolUniform("horizontal", horizontal);
      glBindFramebuffer(GL_FRAMEBUFFER, mPingpongFBO[!horizontal]);
      drawQuad(mGaussianBlurShader, quadVAO, mPingpongColorBuffer[horizontal]);
      horizontal = !horizontal;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mIntermediateFBO);
    mApplyBloomShader.SetActive();
    mApplyBloomShader.SetFloatUniform("exposure", 3.0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mPingpongColorBuffer[!horizontal]);
    drawQuad(mApplyBloomShader, quadVAO, mScreenTexture);

    mTextShader.SetActive();
    const float textColor[3] = {1.0f, 1.0f, 1.0f};
    mTextShader.SetVec3Uniform("textColor", textColor);

    for (auto text : hudTextures) {
      drawToHUD(quadVAO, text, mScreenTexture, textColor);
    }

#ifdef _DEBUG
    // debug marble coordinates
    SDL_Surface *coordinatesSurface = TTF_RenderText_Blended_Wrapped(
        mFont, entities->GetDynamicEntities()[0].GetCoordinatesString().c_str(),
        0, {255, 255, 255, 255}, 0);

    GLuint texture = LoadGLTexture(coordinatesSurface,
                                   mScreenWidth - coordinatesSurface->w - 10,
                                   10, SDL_FLIP_VERTICAL);

    drawToHUD(quadVAO, texture, mScreenTexture, textColor);
    // Prevent textures from flooding GPU mem
    glDeleteTextures(1, &texture);

    // draw shadow map
    glViewport(0, 0, 216, 144);
    mDebugShadowMapShader.SetActive();
    drawQuad(mDebugShadowMapShader, quadVAO, mShadowMapTexture);
    glViewport(0, 0, mScreenWidth, mScreenHeight);
#endif

    if (mShowVictoryMessage)
      drawToHUD(quadVAO, mVictoryTexture, mScreenTexture, textColor);

    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 3. now render quad with scene's visuals as its texture image
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    // draw the result of every other draw
    mPostShader.SetActive();
    drawQuad(mPostShader, quadVAO, mScreenTexture);
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

void Renderer::drawQuad(Shader &shader, GLuint VAO, GLuint texture) {
  glBindVertexArray(VAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  // use the now resolved color attachment as the quad's texture
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Renderer::setView(const shared_ptr<FollowCamera> &camera) {
  mView =
      mat4::CreateLookAt(camera->mActualPosition, camera->mTarget, camera->mUp);
}

void Renderer::setProjection() {
  mProjection = mat4::CreatePerspectiveFOV(
      fovy, static_cast<float>(mScreenWidth), static_cast<float>(mScreenHeight),
      1.0f, 10000.0f);
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
