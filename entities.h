#ifndef ENTITIES_H
#define ENTITIES_H

#include "mesh.h"
#include "pch.h"
#include "physics.h"
#include "template.h"

using Tmpl8::quat;
using Tmpl8::vec3;

inline stack<GLuint> gTo_render_as_collided;
inline vector<TriangleCollider> gCurrent_partition;

enum class BodyType { Dynamic, Static };

struct DynamicEntity {
  Mesh::Mesh mesh;
  Body body;
  SphereCollider collider;
};

struct StaticEntity {
  Mesh::Mesh mesh;
  Body body;
};

class Entities {
  vector<StaticEntity> mStaticEntities;
  vector<vector<TriangleCollider>> mStaticColliders;
  vector<DynamicEntity> mDynamicEntities;

public:
  Entities();
  static void UpdateBody(float t, float dt, DynamicEntity &e,
                         const SpacePartition &sp);
  void Update(float time, float deltaTime);
  void RegisterEntities(const vector<pair<string, BodyType>> &entityList);
  Body &ProvideCameraFollow();
  const vector<StaticEntity> &GetStaticEntities() { return mStaticEntities; }
  const vector<DynamicEntity> &GetDynamicEntities() { return mDynamicEntities; }
};

#endif // ENTITIES_H
