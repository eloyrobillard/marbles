#include "renderer.hpp"

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
void GL_Enter2DMode() {
  glDisable(GL_DEPTH_TEST);

  /* This allows alpha blending of 2D textures with the scene */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GL_Leave2DMode() {
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

static string gp_cSeverity[] = {"High", "Medium", "Low", "Notification"};
static string gp_cType[] = {"Error",       "Deprecated",  "Undefined",
                            "Portability", "Performance", "Other"};
static string gp_cSource[] = {"OpenGL",    "OS",          "GLSL Compiler",
                              "3rd Party", "Application", "Other"};

void debugCallback(uint32_t uiSource, uint32_t uiType, uint32_t uiID,
                   uint32_t uiSeverity, int32_t iLength, const char *p_cMessage,
                   void *p_UserParam) {
  // Get the severity
  uint32_t uiSevID = 3;
  switch (uiSeverity) {
  case GL_DEBUG_SEVERITY_HIGH:
  case GL_DEBUG_SEVERITY_MEDIUM:
  case GL_DEBUG_SEVERITY_LOW:
    uiSevID = uiSeverity - GL_DEBUG_SEVERITY_HIGH;
    break;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
  default:
    uiSevID = 3;
    break;
  }

  // Get the type
  uint32_t uiTypeID = std::min(uiType - GL_DEBUG_TYPE_ERROR, (uint32_t)5);

  // Get the source
  uint32_t uiSourceID = std::min(uiSource - GL_DEBUG_SOURCE_API, (uint32_t)5);

  // Output to the Log
  SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
                  "OpenGL Debug: Severity=%s, Type=%s, Source=%s - %s",
                  gp_cSeverity[uiSevID].c_str(), gp_cType[uiTypeID].c_str(),
                  gp_cSource[uiSourceID].c_str(), p_cMessage);
  if (uiSeverity == GL_DEBUG_SEVERITY_HIGH) {
    // This a serious error so we need to shutdown the program
    SDL_Event event;
    event.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&event);
  }
}

void GLDebug_Init() {
  // Allow for synchronous callbacks.
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

  // Set up the debug info callback
  glDebugMessageCallback((GLDEBUGPROC)&debugCallback, nullptr);

  // Set up the type of debug information we want to receive
  uint32_t uiUnusedIDs = 0;
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                        &uiUnusedIDs, GL_TRUE); // Enable all
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                        GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
                        GL_FALSE); // Disable notifications
}

Renderer::Renderer(bool goFullscreen, int screenWidth, int screenHeight) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

  // Set OpenGL attributes
  // Use the core OpenGL profile
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  // Specify version 4.3
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
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
#ifdef _DEBUG
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

  mIsFullscreen = goFullscreen;
  if (goFullscreen) {
    mWindow = SDL_CreateWindow("Marbles", screenWidth, screenHeight,
                               SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
  } else {
    mWindow = SDL_CreateWindow("Marbles", screenWidth, screenHeight,
                               SDL_WINDOW_OPENGL);
  }

  // Capture mouse
  SDL_SetWindowRelativeMouseMode(mWindow, true);
  // Fix mouse position
  SDL_Rect r = {0, 0, 0, 0};
  SDL_SetWindowMouseRect(mWindow, &r);

  // Save final screen width/height
  SDL_GetWindowSize(mWindow, &mScreenWidth, &mScreenHeight);

  mAspectRatio =
      static_cast<float>(mScreenHeight) / static_cast<float>(mScreenWidth);

  setProjection();

  mGlContext = SDL_GL_CreateContext(mWindow);

  GLenum status = glewInit();

  if (status != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(status));
  }

  printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

#ifdef _DEBUG
  // Initialise debug callback (OpenGL 4.3+)
  GLDebug_Init();
#endif

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
  mWireframeShader =
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
  mDrawStaticShader.SetIntUniform("shadowMapStatic", 2);
  mDrawStaticShader.SetIntUniform("shadowMapMarble", 3);

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
}

void Renderer::Init(const shared_ptr<const Entities> &entities) {
  // Setup HUD
  if (!TTF_Init()) {
    SDL_Log("TTF_Init error: %s\n", SDL_GetError());
  }

  mFont = TTF_GetFont("assets/fonts/FiraCodeNerdFontMono-Regular.ttf", 30,
                      TTF_STYLE_NORMAL);

  // Length can be zero for null-terminated text
  SDL_Surface *commandsSurface =
      TTF_RenderText_Blended_Wrapped(mFont,
                                     "Left/Right arrows to turn\nSpace to "
                                     "split/join marbles\nEnter to restart",
                                     0, {255, 255, 255, 255}, 0);

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
  GL_Enter2DMode();
  float textColor[3] = {1.0f, 1.0f, 1.0f};
  drawToHUD(quadVAO, mLoadingTexture, mScreenTexture, textColor);
  GL_Leave2DMode();

  // Font size for marble coordinates HUD
  TTF_SetFontSize(mFont, 30);

  // Setup light for shadow mapping
  float near = 1.0f, far = 100.0f;
  const vec3 lightPos{60.0f, -3.0f, -15.0f};
  const vec3 lightTarget{60.0f, 0.0f, -17.0f};
  const mat4 lightView = mat4::CreateLookAt(lightPos, lightTarget, vec3::up);
  const mat4 lightProj = mat4::CreateOrtho(100, 100 * mAspectRatio, near, far);
  mLightViewProjStatic = lightView * lightProj;

  glPolygonOffset(1.75f, 0.0f);
  glEnable(GL_POLYGON_OFFSET_FILL);

  // Prepare shadow map
  glBindFramebuffer(GL_FRAMEBUFFER, mStaticShadowMapFBO);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, 8 * mScreenWidth, 8 * mScreenHeight);
  drawSceneWithShader(mShadowMapShader, entities, mLightViewProjStatic);
  glViewport(0, 0, mScreenWidth, mScreenHeight);

  glDisable(GL_POLYGON_OFFSET_FILL);

  SDL_GL_SwapWindow(mWindow);
}

#ifdef _DEBUG
void Renderer::drawMarbleGizmoLikeThing(const DynamicBody &marble,
                                        const mat4 &viewProj) {
  GLuint VAs[3];
  GLuint VBs[3];
  glGenVertexArrays(3, VAs);
  glGenBuffers(3, VBs);

  float verts[3][6] = {
      {marble.position.x, marble.position.y, marble.position.z,
       marble.velocity.x, marble.velocity.y, marble.velocity.z},
      {marble.position.x, marble.position.y, marble.position.z,
       marble.rotationAxis.x, marble.rotationAxis.y, marble.rotationAxis.z},
      {marble.position.x, marble.position.y, marble.position.z, 0.f, 0.f, 1.f}};

  for (auto &vert : verts) {
    vert[3] += vert[0];
    vert[4] += vert[1];
    vert[5] += vert[2];
  }

  float color[3][3] = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};

  mWireframeShader.SetActive();
  mWireframeShader.SetMatrixUniform("uModel", mat4::identity());
  mWireframeShader.SetMatrixUniform("uViewProj", viewProj);

  glDisable(GL_DEPTH_TEST);

  for (int i = 0; i < 3; i++) {
    glBindVertexArray(VAs[i]);
    glBindBuffer(GL_ARRAY_BUFFER, VBs[i]);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), &verts[i], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    mWireframeShader.SetVec3Uniform("tint", color[i]);
    glDrawArrays(GL_LINE_STRIP, 0, 2);
  }

  glEnable(GL_DEPTH_TEST);

  glDeleteVertexArrays(3, VAs);
  glDeleteBuffers(3, VBs);
}
#endif

void Renderer::drawDynamicEntity(const Shader &shader, const DynamicBody &body,
                                 const Mesh &mesh, const mat4 &viewProj) {
  mat4 model = body.getWorldTransform();
  shader.SetMatrixUniform("uWorldTransform", model);

  auto maybeTex = mesh.lookTextureUp(0);
  if (maybeTex.has_value()) {
    glActiveTexture(GL_TEXTURE0);
    maybeTex.value()->SetActive();
  }

  Shader::SetVerticesActive(mesh.GetVertexArray());

  // Draw triangles
  glDrawElements(GL_TRIANGLES, static_cast<int>(mesh.GetNumIndices()),
                 GL_UNSIGNED_INT, nullptr);

  GLenum err_code = glGetError();
  while (GL_NO_ERROR != err_code) {
    printf("OpenGL Error @ %s: %i", "mesh draw", err_code);
    err_code = glGetError();
  }
}

void Renderer::drawPivotEntity(const Shader &shader, const PivotEntity &entity,
                               const mat4 &viewProj) {
  const auto &[worldTransform, maybeTex, vertexArray, numIndices] =
      entity.GetDrawData();
  shader.SetMatrixUniform("uWorldTransform", worldTransform);

  if (maybeTex.has_value()) {
    glActiveTexture(GL_TEXTURE0);
    maybeTex.value()->SetActive();
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

#ifdef _DEBUG
void Renderer::drawDoorNormal(const PivotEntity &entity, const mat4 &viewProj) {
  GLuint VA;
  GLuint VB;
  glGenVertexArrays(1, &VA);
  glGenBuffers(1, &VB);

  float vert[6] = {entity.body.position.x,       entity.body.position.y,
                   entity.body.position.z,       entity.colliders[0].normal.x,
                   entity.colliders[0].normal.y, entity.colliders[0].normal.z};

  vert[3] += vert[0];
  vert[4] += vert[1];
  vert[5] += vert[2];

  float color[3] = {1.f, 0.f, 0.f};

  mWireframeShader.SetActive();
  mWireframeShader.SetMatrixUniform("uModel", mat4::identity());
  mWireframeShader.SetMatrixUniform("uViewProj", viewProj);

  glDisable(GL_DEPTH_TEST);

  glBindVertexArray(VA);
  glBindBuffer(GL_ARRAY_BUFFER, VB);
  glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), vert, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

  mWireframeShader.SetVec3Uniform("tint", color);
  glDrawArrays(GL_LINE_STRIP, 0, 2);

  glEnable(GL_DEPTH_TEST);

  glDeleteVertexArrays(1, &VA);
  glDeleteBuffers(1, &VB);
}
#endif

void Renderer::drawStaticEntity(const Shader &shader,
                                const StaticEntity &entity) {
  const auto &[worldTransform, maybeTex, vertexArray, numIndices] =
      entity.GetDrawData();
  shader.SetMatrixUniform("uWorldTransform", worldTransform);

  if (maybeTex.has_value()) {
    glActiveTexture(GL_TEXTURE0);
    maybeTex.value()->SetActive();
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

Renderer::~Renderer() {
  mMeshShader.Unload();
  mWireframeShader.Unload();
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
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

void configureShadowMap(GLuint &fbo, GLuint &texture, int width, int height) {
  // configure shadow map framebuffer
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // create texture for shadow map
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Anything outside of shadow map should not be in shadow
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         texture, 0);
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

  // framebuffer to be reused while rendering entity outlines
  glGenFramebuffers(1, &mContourFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, mContourFBO);
  mContourColorBuffer =
      createColorAttachmentTexture(mScreenWidth, mScreenHeight, GL_RGB);

  // configure shadow map for static elements
  configureShadowMap(mStaticShadowMapFBO, mStaticShadowMapTexture,
                     8 * mScreenWidth, 8 * mScreenHeight);
  // configure shadow map for marble area
  configureShadowMap(mMarbleShadowMapFBO, mMarbleShadowMapTexture, mScreenWidth,
                     mScreenHeight);

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
    cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete "
            "!\n";

  // Used for Gaussian blur (bloom effect)
  glGenFramebuffers(2, mPingpongFBO);
  // Textures -> no need for depth buffer
  glGenTextures(2, mPingpongColorBuffer);
  for (uint i = 0; i < 2; i++) {
    glBindFramebuffer(GL_FRAMEBUFFER, mPingpongFBO[i]);
    glBindTexture(GL_TEXTURE_2D, mPingpongColorBuffer[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mScreenWidth, mScreenHeight, 0,
                 GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           mPingpongColorBuffer[i], 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      cout << "ERROR::FRAMEBUFFER:: Ping-pong framebuffer is not complete !\n";
  }

  return (glGetError() == 0);
}

void Renderer::drawCollisionDebug(const mat4 &viewProj) {
  // override previous triangle draw no matter what
  glDisable(GL_DEPTH_TEST);

  // Visualize collisions
  mCollisionShader.SetActive();

  mCollisionShader.SetMatrixUniform("uViewProj", viewProj);
  mCollisionShader.SetMatrixUniform("uModel", mat4::identity());
  mCollisionShader.SetVec3Uniform("tint", vec3(1.f, 0.6f, 0.f));

  while (!gRenderAsCollided.empty()) {
    Shader::SetVerticesActive(gRenderAsCollided.top());

    // Draw triangles
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

    gRenderAsCollided.pop();
  }

  while (!gRenderDoorAsCollided.empty()) {
    auto [va, model] = gRenderDoorAsCollided.top();

    mCollisionShader.SetMatrixUniform("uModel", model);

    Shader::SetVerticesActive(va);

    // Draw triangles
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

    gRenderDoorAsCollided.pop();
  }

  mCollisionShader.SetVec3Uniform("tint", vec3(0.f, 0.f, 1.f));
  mCollisionShader.SetMatrixUniform("uModel", mat4::identity());

  while (!gShowRaycastHit.empty()) {
    Shader::SetVerticesActive(gShowRaycastHit.top());

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

    gShowRaycastHit.pop();
  }

  glEnable(GL_DEPTH_TEST);

  // Visualize triangle colliders as a wireframe
  mWireframeShader.SetActive();

  mWireframeShader.SetMatrixUniform("uModel", mat4::identity());

  mWireframeShader.SetMatrixUniform("uViewProj", viewProj);
  mWireframeShader.SetVec3Uniform("tint", {0.f, 1.f, 0.f});

  // Turn on wireframe mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Only show triangles currently tested against
  for (const auto &vertexArray : gShowWireframe) {
    Shader::SetVerticesActive(vertexArray);

    // Draw triangles
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
  }

  for (const auto &[va, model] : gShowDoorWireframe) {
    Shader::SetVerticesActive(va);

    mWireframeShader.SetMatrixUniform("uModel", model);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
  }

  mWireframeShader.SetVec3Uniform("tint", {0.f, 0.f, 1.f});
  mWireframeShader.SetMatrixUniform("uModel", mat4::identity());

  while (!gShowRaycastWireframe.empty()) {
    Shader::SetVerticesActive(gShowRaycastWireframe.top());

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

    gShowRaycastWireframe.pop();
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
    drawStaticEntity(shader, e);
  }

  const auto &[marbleMesh, marbles, numMarbles] =
      entities->GetDynamicEntities();
  for (int i = 0; i < numMarbles; i++) {
    drawDynamicEntity(shader, marbles[i], marbleMesh, viewProj);
  }
}

void Renderer::drawScene(const shared_ptr<const Entities> &entities,
                         const mat4 &viewProj, const vec3 &lightDir,
                         const mat4 &lightViewProj, float near, float far) {
  glBindFramebuffer(GL_FRAMEBUFFER, mDepthMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  mDepthMapShader.SetActive();
  mDepthMapShader.SetMatrixUniform("uViewProj", viewProj);

  for (const auto &e : entities->GetStaticEntities()) {
    drawStaticEntity(mDepthMapShader, e);
  }

  for (const auto &e : entities->GetDoors()) {
    drawPivotEntity(mDrawStaticShader, e, viewProj);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, mContourFBO);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mContourShader.SetActive();
  drawQuad(mContourShader, quadVAO, mDepthMapTexture);

  glBindFramebuffer(GL_FRAMEBUFFER, mMSAAFBO);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mDrawStaticShader.SetActive();
  mDrawStaticShader.SetVec3Uniform("lightDir", lightDir);
  mDrawStaticShader.SetFloatUniform("near", near);
  mDrawStaticShader.SetFloatUniform("far", far);
  mDrawStaticShader.SetMatrixUniform("viewProj", viewProj);
  mDrawStaticShader.SetMatrixUniform("lightViewProjStatic",
                                     mLightViewProjStatic);
  mDrawStaticShader.SetMatrixUniform("lightViewProjMarble", lightViewProj);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, mContourColorBuffer);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, mStaticShadowMapTexture);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, mMarbleShadowMapTexture);

  for (const auto &e : entities->GetStaticEntities()) {
    drawStaticEntity(mDrawStaticShader, e);
  }

  for (const auto &e : entities->GetDoors()) {
    drawPivotEntity(mDrawStaticShader, e, viewProj);
  }

#ifdef _DEBUG
  drawCollisionDebug(viewProj);

  for (const auto &e : entities->GetDoors()) {
    drawDoorNormal(e, viewProj);
  }
#endif

  mMeshShader.SetActive();
  mMeshShader.SetMatrixUniform("uViewProj", viewProj);

  const auto &[marbleMesh, marbles, numMarbles] =
      entities->GetDynamicEntities();
  for (int i = 0; i < numMarbles; i++) {
    drawDynamicEntity(mMeshShader, marbles[i], marbleMesh, viewProj);
  }

#ifdef _DEBUG
  for (int i = 0; i < numMarbles; i++) {
    drawMarbleGizmoLikeThing(marbles[i], viewProj);
  }
#endif
}

void Renderer::Draw3D(float deltaTime, const shared_ptr<Entities> &entities,
                      ICamera &camera) {
  setView(camera);

  glEnable(GL_DEPTH_TEST);

  mat4 viewProj = mView * mProjection;

  // Setup light for shadow mapping
  float near = 1.0f, far = 100.0f;
  const vec3 &lightTarget = entities->ProvideCameraFollow();
  const vec3 lightPos{lightTarget.x, lightTarget.y - 3.0f,
                      lightTarget.z + 2.0f};
  const mat4 lightView = mat4::CreateLookAt(lightPos, lightTarget, vec3::up);
  const mat4 lightProj = mat4::CreateOrtho(5, 5 * mAspectRatio, near, far);
  const mat4 lightViewProj = lightView * lightProj;

  // Prepare shadow map
  glEnable(GL_POLYGON_OFFSET_FILL);
  glBindFramebuffer(GL_FRAMEBUFFER, mMarbleShadowMapFBO);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawSceneWithShader(mShadowMapShader, entities, lightViewProj);
  glDisable(GL_POLYGON_OFFSET_FILL);

  // Draw the final scene, with shadow + bloom + contours
  drawScene(entities, viewProj, lightTarget - lightPos, lightViewProj, near,
            far);

  // finally, draw HUD elements
  GL_Enter2DMode();
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Blit multisampled buffer(s) to normal colorbuffer of intermediate FBO.
    // Image is stored in screenTexture
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
        mFont, entities->GetDynamicEntitiesCoordinates().c_str(), 0,
        {255, 255, 255, 255}, 0);

    GLuint texture = LoadGLTexture(coordinatesSurface,
                                   mScreenWidth - coordinatesSurface->w - 10,
                                   10, SDL_FLIP_VERTICAL);

    SDL_DestroySurface(coordinatesSurface);

    drawToHUD(quadVAO, texture, mScreenTexture, textColor);
    // Prevent textures from flooding GPU mem
    glDeleteTextures(1, &texture);

    // draw shadow map
    glViewport(0, 0, 216, static_cast<int>(216 * mAspectRatio));
    mDebugShadowMapShader.SetActive();
    drawQuad(mDebugShadowMapShader, quadVAO, mStaticShadowMapTexture);
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
  GL_Leave2DMode();

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

void Renderer::setView(ICamera &camera) {
  mView =
      mat4::CreateLookAt(camera.GetPosition(), camera.GetTarget(), vec3::up);
}

void Renderer::setProjection() {
  mProjection = mat4::CreatePerspectiveFOV(
      fovy, static_cast<float>(mScreenWidth), static_cast<float>(mScreenHeight),
      1.0f, 100.0f);
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
