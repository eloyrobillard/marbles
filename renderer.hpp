#ifndef RENDERER_H
#define RENDERER_H

#include "camera.hpp"
#include "entities.hpp"
#include "maths.hpp"
#include "pch.h"

#define GLEW_BUILD
extern "C" {
#include "glew.h"
}
#include "gl.h"

using Maths::mat4;
using Maths::PI;

class Renderer {
  mat4 mView;
  mat4 mProjection;
  float fovy = 30.0f / 180.0f * PI;
  int mScreenWidth;
  int mScreenHeight;
  // Screen height divided by screen width
  float mAspectRatio;
  bool mIsFullscreen;
  TTF_Font *mFont;

  SDL_Window *mWindow;
  SDL_GLContext mGlContext;
  shared_ptr<FollowCamera> mCamera;

  vector<GLuint> hudTextures;

  GLuint mMarbleShadowMapFBO;
  GLuint mMarbleShadowMapTexture;
  GLuint mStaticShadowMapFBO;
  GLuint mStaticShadowMapTexture;
  // View-projection for light used in shadow map for static elements
  mat4 mLightViewProjStatic;
  GLuint mDepthMapFBO;
  GLuint mContourFBO;
  GLuint mContourColorBuffer;
  GLuint mMSAAFBO;
  GLuint mMSAARenderBuffer;
  GLuint mIntermediateFBO;
  GLuint mScreenTexture;
  GLuint mBrightnessTexture;
  // Used for Gaussian blur (bloom effect)
  GLuint mPingpongFBO[2];
  GLuint mPingpongColorBuffer[2];
  GLuint quadVAO, quadVBO;

  GLuint mLoadingTexture;
  GLuint mVictoryTexture;
  GLuint mDepthMapTexture;

  Shader mDebugShadowMapShader;
  Shader mDepthMapShader;
  Shader mShadowMapShader;
  Shader mContourShader;
  Shader mDrawStaticShader;
  Shader mTextShader;
  Shader mMeshShader;
  Shader mColliderShader;
  Shader mCollisionShader;
  Shader mPostShader;
  Shader mGaussianBlurShader;
  Shader mApplyBloomShader;

  bool mShowVictoryMessage = false;

  bool setupFramebuffers();

  void setView(const shared_ptr<FollowCamera> &camera);
  void setProjection();
  void pushHUDTexture(GLuint texture) { hudTextures.push_back(texture); }
  void getMeshes(const vector<pair<string, BodyType>> &meshList);
  void drawCollisionDebug(const mat4 &viewProj);
  void drawToHUD(GLuint VAO, GLuint textTexture, GLuint screenTexture,
                 const float textColor[3]);
  void configureMultiSampledAntiAliasing();
  void prepareShadowMap(const shared_ptr<const Entities> &entities,
                        const mat4 &viewProj, const mat4 &lightViewProj);
  // Load an SDL surface into OpenGL as a screen-sized texture
  GLuint LoadGLTexture(SDL_Surface *surface, int dst_x, int dst_y,
                       SDL_FlipMode flip_mode) const;

  void drawSceneWithShader(const Shader &shader,
                           const shared_ptr<const Entities> &entities,
                           const mat4 &viewProj);
  void drawScene(const shared_ptr<const Entities> &entities,
                 const mat4 &viewProj, const vec3 &lightDir,
                 const mat4 &lightViewProj, float near, float far);
  static void setupQuadVAO(GLuint &VAO, GLuint &VBO);
  static Shader GetShader(const char *vert, const char *frag);
  static void drawQuad(Shader &shader, GLuint VAO, GLuint texture);
  static void blitFramebuffer(GLuint readFB, GLuint drawFB, int readW,
                              int readH, int drawW, int drawH);
  void drawEntity(const Shader &shader, const Entity &entity);
  static GLuint createColorAttachmentTexture(
      int width, int height, int colorFormat,
      int colorAttachmentNumber = GL_COLOR_ATTACHMENT0);

public:
  Renderer(bool goFullscreen = false, int screenWidth = 1280,
           int screenHeight = 720);
  ~Renderer();
  // Setup HUD and global shadow map
  void Init(const shared_ptr<const Entities> &entities);
  void Draw3D(float deltaTime, const shared_ptr<Entities> &entities);
  void SetCamera(const shared_ptr<FollowCamera> &camera);
  void ShowVictoryMessage() { mShowVictoryMessage = true; }
  void ToCheckpoint() { mShowVictoryMessage = false; }
};

#endif
