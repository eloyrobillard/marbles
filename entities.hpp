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
inline vector<TriangleCollider> gCurrentPartition;

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

  Mesh mesh;
  Body body;
};

class DynamicEntity : public Entity {
  vec3 mPrevPos;

public:
  SphereCollider collider;

  DynamicEntity(const Mesh &mesh, const Body &body, SphereCollider collider)
      : Entity(mesh, body), collider(collider) {}
  void UpdateFirstPass(float t, float dt);
  void UpdateSecondPass(float t, float dt);
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
  [[nodiscard]] vec3 &GetVelocityAsRef() { return body.velocity; }
  void SetPosition(const vec3 &pos) { body.position = pos; }
  void SetVelocity(const vec3 &vel) { body.velocity = vel; }
};

// Container for all game objects
class Entities {
  vector<Entity> mStaticEntities;
  vector<vector<TriangleCollider>> mStaticColliders;
  vector<DynamicEntity> mDynamicEntities;

  // HACK: Not even correct: this works despite missing `take_view`
  ranges::subrange<
      ranges::iterator_t<
          ranges::drop_view<ranges::ref_view<vector<DynamicEntity>>>>,
      ranges::sentinel_t<
          ranges::drop_view<ranges::ref_view<vector<DynamicEntity>>>>>
      mCurrentDynamicEntities;

  vector<DynamicEntity> mDynamicEntitiesStartingState;
  bool mSplitMode = false;

public:
  Entities();
  void Update(float time, float deltaTime);
  void RegisterEntities(const vector<EntityData> &entityList);
  [[nodiscard]]
  const DynamicEntity &ProvideCameraFollow() const {
    return mCurrentDynamicEntities[0];
  }

  [[nodiscard]] const vector<Entity> &GetStaticEntities() const {
    return mStaticEntities;
  }

  [[nodiscard]] auto GetDynamicEntities() const {
    return mCurrentDynamicEntities;
  }

  string GetDynamicEntitiesCoordinates() {
    return mCurrentDynamicEntities[0].GetCoordinatesString();
  }
  // TODO: move input handling somewhere else
  void RegisterInputForward(float dt);
  void RegisterInputLeft(float dt);
  void RegisterInputRight(float dt);
  void ToCheckpoint(const vector<vec3> &positionsAtCheckpoint);
  void ToggleSplitMode();

  void GetCollisionImpulse();
};

#endif // ENTITIES_H
