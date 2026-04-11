#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "mesh.h"
#include "pch.h"
#include "surface.h"
#include "template.h"

using Tmpl8::mat4;
using Tmpl8::PI;
using Tmpl8::Surface;

enum class BodyType { Dynamic, Static };

class Renderer {
  mat4 mView;
  mat4 mProjection;
  float fovy = 30.0f / 180.0f * PI;
  deque<Mesh::Mesh> mMeshes;

  Shader::Shader mMeshShader;
  Shader::Shader mColliderShader;
  Shader::Shader mCollisionShader;

public:
  Renderer(const unique_ptr<FollowCamera> &camera, Surface *screen);
  ~Renderer();
  void Draw3D(float deltaTime, const unique_ptr<FollowCamera> &camera);
  void SetView(const unique_ptr<FollowCamera> &camera);
  void SetProjection(Surface *screen);
  void GetMeshes(const vector<pair<string, BodyType>> &meshList);
  static Shader::Shader GetShader(const char *vert, const char *frag);
};

#endif
