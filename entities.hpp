#ifndef ENTITIES_H
#define ENTITIES_H

#include "colliders.hpp"
#include "maths.hpp"
#include "mesh.hpp"
#include "pch.h"

using Maths::quat;
using Maths::vec3;
using Maths::vec4;

inline stack<GLuint> gToRenderAsCollided;
inline stack<pair<GLuint, mat4>> gToRenderDoorAsCollided;

inline vector<GLuint> gShowWireframe;

inline vector<pair<GLuint, mat4>> gShowDoorWireframe;

struct DynamicEntityData {
  string meshPath;
  vec3 scale = vec3(1.0f);
};

struct DoorData {
  string meshPath;
  vec3 scale = vec3(1.0f);
  vec3 pivotAxis;
  // Point in local coordinates
  vec3 pivotPoint;
  pair<float, float> angleBoundsDeg;
  // Resisting force pushing the door back close
  float resistance;
};

struct StaticEntityData {
  string meshPath;
  float collisionAcceleration = 1.0f;
  bool overrideSpeed = false;
  // Rotation of override is local to mesh
  // e.g. vec3::forward would be the mesh's forward direction
  vec3 speedOverride = vec3::zero;
  bool overrideImpulse = false;
  // Rotation of override is local to mesh
  // e.g. vec3::forward would be the mesh's forward direction
  vec3 impulseOverride = vec3::zero;
  vec3 scale = vec3(1.0f);
};

struct Entity {
  Entity(Mesh mesh) : mesh(std::move(mesh)) {}

  Mesh mesh;
};

struct PivotEntity : Entity {
  PivotEntity(const Mesh &mesh, const shared_ptr<PivotBody> &body,
              vector<TriangleCollider<PivotBody>> &colliders)
      : Entity(mesh), body(body), colliders(std::move(colliders)) {}

  [[nodiscard]] string GetCoordinatesString() const {
    return std::format("x: {:8.3f}\ny: {:8.3f}\nz: {:8.3f}", body->position.x,
                       body->position.y, body->position.z);
  }

  [[nodiscard]] tuple<mat4, optional<Texture *>, GLuint, size_t>
  GetDrawData() const {
    return {body->getWorldTransform(), mesh.lookTextureUp(0),
            mesh.GetVertexArray(), mesh.GetNumIndices()};
  }

  shared_ptr<PivotBody> body;
  vector<TriangleCollider<PivotBody>> colliders;
};

struct StaticEntity : Entity {
  StaticEntity(const Mesh &mesh, const shared_ptr<StaticBody> &body,
               vector<TriangleCollider<StaticBody>> &colliders)
      : Entity(mesh), body(body), colliders(std::move(colliders)) {}

  [[nodiscard]] string GetCoordinatesString() const {
    return std::format("x: {:8.3f}\ny: {:8.3f}\nz: {:8.3f}", body->position.x,
                       body->position.y, body->position.z);
  }

  [[nodiscard]] tuple<mat4, optional<Texture *>, GLuint, size_t>
  GetDrawData() const {
    return {body->getWorldTransform(), mesh.lookTextureUp(0),
            mesh.GetVertexArray(), mesh.GetNumIndices()};
  }

  shared_ptr<StaticBody> body;
  vector<TriangleCollider<StaticBody>> colliders;
};

struct DynamicEntity : public Entity {
  vec3 mPrevPos;
  SphereCollider collider;
  DynamicBody body;

  [[nodiscard]] string GetCoordinatesString() const {
    return std::format("x: {:8.3f}\ny: {:8.3f}\nz: {:8.3f}", body.position.x,
                       body.position.y, body.position.z);
  }

  DynamicEntity(const Mesh &mesh, const DynamicBody &body,
                SphereCollider &collider)
      : Entity(mesh), body(body), collider(collider) {}
  void UpdateFirstPass(float t, float dt);
  void UpdateSecondPass(float t, float dt);
  [[nodiscard]] const vec3 &GetPositionAsRef() const { return body.position; }
  [[nodiscard]] vec3 &GetVelocityAsRef() { return body.velocity; }
  void SetPosition(const vec3 &pos) { body.position = pos; }
  void SetVelocity(const vec3 &vel) { body.velocity = vel; }
};

enum class SplitMode { Split, Joining, Joined };

// Container for all game objects
class Entities {
  vector<StaticEntity> mStaticEntities;
  vector<PivotEntity> mDoors;

  Mesh mMarbleMesh;
  vector<DynamicBody> mMarbles;
  vector<DynamicBody> mDynamicEntitiesStartingState;
  vector<vec3> mPreviousPositions;

  int mMaxNumMarbles = 6;
  int mCurNumMarbles = 6;
  vec3 mAveragePos;
  float mPositionalVariance = 0.0f;
  vec3 mAverageVel;
  SplitMode mSplitMode = SplitMode::Joined;

  void split();
  void join();
  void startJoin() { mSplitMode = SplitMode::Joining; }
  void stopJoin() { mSplitMode = SplitMode::Split; }

  void computeAveragePosition();
  void computeAveragePositionWithoutOutliers();
  void computePositionalVariance();
  void computeAverageVelocity();

  [[nodiscard]] int getNumMarbles() const {
    return mSplitMode == SplitMode::Joined ? 1 : mCurNumMarbles;
  }

  static void updateFirstPass(DynamicBody &body, float t, float dt);
  static void updateSecondPass(DynamicBody &body, float t, float dt);

public:
  Entities();
  void Update(float time, float deltaTime);
  // Register the mesh/textures for the marbles.
  // Only one mesh/textures set can be active at a time.
  void RegisterMarble(const DynamicEntityData &entity);
  void RegisterDoor(const DoorData &doorData);
  void RegisterStaticEntities(const vector<StaticEntityData> &data);

  [[nodiscard]]
  vec3 ProvideCameraFollow() const {
    return mAveragePos;
  }

  [[nodiscard]]
  vec3 ProvideCameraForward() const {
    return mAverageVel.normalized();
  }

  [[nodiscard]] const vector<PivotEntity> &GetDoors() const { return mDoors; }

  [[nodiscard]] const vector<StaticEntity> &GetStaticEntities() const {
    return mStaticEntities;
  }

  [[nodiscard]] tuple<Mesh, vector<DynamicBody>, int>
  GetDynamicEntities() const {
    return {mMarbleMesh, mMarbles, getNumMarbles()};
  }

  string GetDynamicEntitiesCoordinates() {
    return std::format("x: {:8.3f}\ny: {:8.3f}\nz: {:8.3f}", mAveragePos.x,
                       mAveragePos.y, mAveragePos.z);
  }

  // TODO: move input handling somewhere else
  void RegisterInputForward(float dt);
  void RegisterInputBackward(float dt);
  void RegisterInputLeft(float dt);
  void RegisterInputRight(float dt);
  void ToCheckpoint(const vector<vec3> &positionsAtCheckpoint);
  void ToggleSplitMode();

  void GetDynamicCollisionImpulse();
  void GetStaticCollisionImpulse();
};

#endif // ENTITIES_H
