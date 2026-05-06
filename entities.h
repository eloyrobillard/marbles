#ifndef ENTITIES_H
#define ENTITIES_H

#include "maths.hpp"
#include "mesh.h"
#include "pch.h"
#include "physics.h"
#include "shader.h"

using Maths::quat;
using Maths::vec3;

inline stack<GLuint> gTo_render_as_collided;
inline vector<TriangleCollider> gCurrent_partition;

enum class BodyType { Dynamic, Static };

// NOTE: As per C.2 section from the C++ Core Guidelines
// Making Entity a struct to make it clear mesh and body can vary independently
struct Entity {
  Entity(Mesh mesh, Body body) : mesh(std::move(mesh)), body(std::move(body)) {}
  void Draw(const Shader &shader) const;

protected:
  Mesh mesh;
  Body body;
};

class DynamicEntity : public Entity {
  SphereCollider collider;

public:
  DynamicEntity(const Mesh &mesh, const Body &body, SphereCollider collider)
      : Entity(mesh, body), collider(collider) {}
  void Update(float t, float dt, const SpacePartition &sp);
  void ResetToPosition(const vec3 &pos) {
    body.position = pos;
    body.velocity = vec3::zero;
    body.rotational_velocity = vec3::zero;
  }
  void RegisterInputLeft(float dt) {
    vec3 left = body.velocity.cross(vec3::up).normalized();
    body.velocity += left * 4.0f * dt;
  }
  void RegisterInputRight(float dt) {
    vec3 right = vec3::up.cross(body.velocity).normalized();
    body.velocity += right * 4.0f * dt;
  }
  [[nodiscard]] const vec3 &GetPositionAsRef() const { return body.position; }
};

class Entities {
  vector<Entity> mStaticEntities;
  vector<vector<TriangleCollider>> mStaticColliders;
  vector<DynamicEntity> mDynamicEntities;
  vector<DynamicEntity> mDynamicEntitiesStartingState;

public:
  Entities();
  void Update(float time, float deltaTime);
  void RegisterEntities(
      const vector<tuple<string, BodyType, float, bool, vec3>> &entityList);
  [[nodiscard]]
  const vec3 &ProvideCameraFollow() const {
    return mDynamicEntities[0].GetPositionAsRef();
  }
  [[nodiscard]] const vector<Entity> &GetStaticEntities() const {
    return mStaticEntities;
  }
  [[nodiscard]] const vector<DynamicEntity> &GetDynamicEntities() const {
    return mDynamicEntities;
  }
  void RegisterInputForward(float dt);
  void RegisterInputLeft(float dt);
  void RegisterInputRight(float dt);
  void ToCheckpoint(const vector<vec3> &positionsAtCheckpoint);
  void DrawStaticEntities(const Shader &shader) const {
    for (const auto &e : mStaticEntities) {
      e.Draw(shader);
    }
  }

  void DrawDynamicEntities(const Shader &shader) const {
    for (const auto &e : mDynamicEntities) {
      e.Draw(shader);
    }
  }
};

#endif // ENTITIES_H
