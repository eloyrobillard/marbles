#ifndef ENTITIES_H
#define ENTITIES_H

#include "maths.hpp"
#include "mesh.hpp"
#include "pch.h"
#include "physics.hpp"
#include "shader.hpp"

using Maths::quat;
using Maths::vec3;

inline stack<GLuint> gTo_render_as_collided;
inline vector<TriangleCollider> gCurrent_partition;

enum class BodyType { Dynamic, Static };

struct EntityData {
  string meshPath;
  BodyType bodyType = BodyType::Static;
  float collisionAcceleration = 1.0f;
  bool overrideImpulse = false;
  vec3 impulseOverride = vec3::zero;
  vec3 scale = vec3(1.0f);
};

// NOTE: As per C.2 section from the C++ Core Guidelines
// Making Entity a struct to make it clear mesh and body can vary independently
struct Entity {
  Entity(Mesh mesh, Body body) : mesh(std::move(mesh)), body(std::move(body)) {}
  [[nodiscard]] tuple<mat4, optional<Texture *>, GLuint, size_t>
  GetDrawData() const;

  [[nodiscard]] string GetCoordinatesString() const {
    return std::format("x: {:8.3f}\ny: {:8.3f}\nz: {:8.3f}", body.position.x,
                       body.position.y, body.position.z);
  }

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
  [[nodiscard]] const vec3 &GetVelocityAsRef() const { return body.velocity; }
};

// Container for all game objects
class Entities {
  vector<Entity> mStaticEntities;
  vector<vector<TriangleCollider>> mStaticColliders;
  vector<DynamicEntity> mDynamicEntities;
  vector<DynamicEntity> mDynamicEntitiesStartingState;

public:
  Entities();
  void Update(float time, float deltaTime);
  void RegisterEntities(const vector<EntityData> &entityList);
  [[nodiscard]]
  const DynamicEntity &ProvideCameraFollow() const {
    return mDynamicEntities[0];
  }
  [[nodiscard]] const vector<Entity> &GetStaticEntities() const {
    return mStaticEntities;
  }
  [[nodiscard]] const vector<DynamicEntity> &GetDynamicEntities() const {
    return mDynamicEntities;
  }
  // TODO: move input handling somewhere else
  void RegisterInputForward(float dt);
  void RegisterInputLeft(float dt);
  void RegisterInputRight(float dt);
  void ToCheckpoint(const vector<vec3> &positionsAtCheckpoint);
};

#endif // ENTITIES_H
