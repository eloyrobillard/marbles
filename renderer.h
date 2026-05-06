#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "entities.h"
#include "maths.hpp"
#include "pch.h"
#include "surface.h"

#define GLEW_BUILD
extern "C" {
#include "glew.h"
}
#include "gl.h"

using Maths::mat4;
using Maths::PI;
using Tmpl8::Surface;

class Renderer {
  mat4 mView;
  mat4 mProjection;
  float fovy = 30.0f / 180.0f * PI;

  SDL_Window *mWindow;
  SDL_GLContext mGlContext;
  shared_ptr<Surface> mScreen;
  shared_ptr<FollowCamera> mCamera;

  vector<GLuint> hudTextures;

  GLuint skyboxTexture;
  GLuint skyboxVAO, skyboxVBO;
  GLuint framebuffer;
  GLuint rbo;
  GLuint intermediateFBO;
  GLuint screenTexture;
  GLuint quadVAO, quadVBO;
  GLuint hudVAO, hudVBO;
  GLuint mVictoryTexture;

  Shader mTextShader;
  Shader mMeshShader;
  Shader mColliderShader;
  Shader mCollisionShader;
  Shader mSkyboxShader;
  Shader mPostShader;

  bool mShowVictoryMessage = false;

  bool setupFramebuffers();
  bool setupSkyboxVAO();
  bool setupSkybox();

  static void setupScreenQuadVAO(GLuint &VAO, GLuint &VBO);

  void setView(const shared_ptr<FollowCamera> &camera);
  void setProjection(const shared_ptr<Surface> &screen);
  void pushHUDTexture(GLuint texture) { hudTextures.push_back(texture); }
  void getMeshes(const vector<pair<string, BodyType>> &meshList);
  static Shader GetShader(const char *vert, const char *frag);
  void drawSkybox();
  static void drawScreenQuad(Shader &shader, GLuint VAO, GLuint texture);
  static void blitFramebuffer(GLuint readFB, GLuint drawFB, int readW,
                              int readH, int drawW, int drawH);
  void drawDebug(const mat4 &viewProj);
  void drawEntities(const shared_ptr<const Entities> &entities,
                    const mat4 &viewProj);

public:
  Renderer(const shared_ptr<Surface> &screen);
  ~Renderer();
  void Draw3D(float deltaTime, const shared_ptr<const Entities> &entities);
  void SetCamera(const shared_ptr<FollowCamera> &camera);
  void ShowVictoryMessage() { mShowVictoryMessage = true; }
  void Restart() { mShowVictoryMessage = false; }
};

#endif
