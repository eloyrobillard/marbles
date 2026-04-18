#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "entities.h"
#include "pch.h"
#include "surface.h"
#include "template.h"

#define GLEW_BUILD
extern "C" {
#include "glew.h"
}
#include "gl.h"
#include "wglext.h"

using Tmpl8::mat4;
using Tmpl8::PI;
using Tmpl8::Surface;

typedef BOOL(APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);

class Renderer {
  mat4 mView;
  mat4 mProjection;
  float fovy = 30.0f / 180.0f * PI;

  SDL_Window *mWindow;
  SDL_GLContext mGlContext;
  shared_ptr<Surface> mScreen;
  shared_ptr<FollowCamera> mCamera;

  GLuint skyboxTexture;
  GLuint skyboxVAO, skyboxVBO;
  GLuint framebuffer;
  GLuint rbo;
  GLuint intermediateFBO;
  GLuint screenTexture;
  GLuint quadVAO, quadVBO;

  Shader::Shader mMeshShader;
  Shader::Shader mColliderShader;
  Shader::Shader mCollisionShader;
  Shader::Shader mSkyboxShader;
  Shader::Shader mPostShader;

  bool setupFramebuffers();
  bool setupSkyboxVAO();

public:
  Renderer(const shared_ptr<Surface> &screen);
  ~Renderer();
  void Draw3D(float deltaTime, const vector<StaticEntity> &se,
              const vector<DynamicEntity> &de);
  void SetView(const shared_ptr<FollowCamera> &camera);
  void SetProjection(const shared_ptr<Surface> &screen);
  void GetMeshes(const vector<pair<string, BodyType>> &meshList);
  static Shader::Shader GetShader(const char *vert, const char *frag);
  void SetCamera(const shared_ptr<FollowCamera> &camera);
};

#endif
