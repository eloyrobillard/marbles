#ifndef _PHYSICS_H
#define _PHYSICS_H
#pragma once

#include "maths.hpp"
#include "pch.h"

using Maths::mat4;
using Maths::quat;
using Maths::vec3;

const vec3 grav_force = vec3(0.0f, 0.0f, -9.81f);

struct Collider {};

struct SphereCollider : Collider {
  SphereCollider(vec3 position, float radius)
      : position(position), radius(radius) {}

  vec3 position;
  float radius;
};

// For static objects
struct TriangleCollider : Collider {
  TriangleCollider(vec3 normal, vec3 a, vec3 b, vec3 c, float accel,
                   bool overrideImpulse, vec3 impulseOverride,
                   GLuint vertexBuffer, GLuint indexBuffer, GLuint vertexArray)
      : normal(normal), a(a), b(b), c(c), accel(accel),
        overrideImpulse(overrideImpulse), impulseOverride(impulseOverride),
        vertexBuffer(vertexBuffer), indexBuffer(indexBuffer),
        vertexArray(vertexArray) {}

  vec3 a;
  vec3 b;
  vec3 c;
  float accel;
  bool overrideImpulse;
  vec3 impulseOverride;
  vec3 normal;
  GLuint vertexBuffer;
  GLuint indexBuffer;
  GLuint vertexArray;
};

inline ostream &operator<<(ostream &os, const SphereCollider &coll) {
  os << "SphereCollider { radius: " << coll.radius
     << ", position: " << coll.position << " }";
  return os;
}

inline ostream &operator<<(ostream &os, const TriangleCollider &coll) {
  os << "TriangleCollider { a: " << coll.a << ", b: " << coll.b
     << ", c: " << coll.c << ", normal: " << coll.normal << " }";
  return os;
}

class Body {
public:
  vec3 scale;
  vec3 position;
  vec3 velocity;
  quat rotation;
  vec3 rotational_velocity;
  vector<Collider> colliders;

  Body()
      : scale(vec3(1.0f)), position(vec3()), velocity(vec3()), rotation(quat()),
        rotational_velocity(vec3()), colliders({}) {}

  [[nodiscard]] mat4 getWorldTransform() const {
    mat4 s = mat4::CreateScale(scale);
    mat4 r = mat4::CreateFromQuaternion(rotation);
    mat4 t = mat4::CreateTranslation(position);

    return s * r * t;
  }
};

class SpacePartition {
public:
  float mMax_x;
  float mMin_x;
  float mStep;
  vector<vector<TriangleCollider>> mColliders;

  SpacePartition(float min_x, float max_x, float step)
      : mMin_x(min_x), mMax_x(max_x), mStep(step) {
    mColliders = vector<vector<TriangleCollider>>(ceil((max_x - min_x) / step));
  }
  SpacePartition(float min_x, float max_x, float step,
                 const vector<TriangleCollider> &v)
      : mMin_x(min_x), mMax_x(max_x), mStep(step) {
    mColliders = vector<vector<TriangleCollider>>(ceil((max_x - min_x) / step));
    populate(v);
  }
  ~SpacePartition() = default;

  void populate(const TriangleCollider &tc, float min_x, float max_x);
  void populate(const vector<TriangleCollider> &v);
  [[nodiscard]] vector<TriangleCollider>
  get_partition(const SphereCollider &s, float min_x, float max_x) const;

  // friend ostream &operator<<(ostream &os, const SpacePartition &n) {
  //   n.print(os);
  //   return os;
  // }
};

// Used for spatial partitioning of static colliders
inline SpacePartition gSpacePartition = SpacePartition(4.0f, 150.0f, 1.0f);

namespace Physics {
bool getCollisionImpulse(const SpacePartition &sp,
                         const SphereCollider &collider, vec3 &velocity);

inline float physicsTicksPerSecond = 120.0;
inline float physicsDeltaTime = 1.0f / physicsTicksPerSecond;
} // namespace Physics

#endif // _PHYSICS_H
