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
  SphereCollider(vec3 &position, float radius)
      : position(position), radius(radius) {}

  vec3 position;
  float radius;
};

// For static objects
struct TriangleCollider : Collider {
  TriangleCollider(vec3 &normal, vec3 &a, vec3 &b, vec3 &c, float accel,
                   bool overrideImpulse, vec3 &impulseOverride,
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
  vec3 scale = vec3(1.0f);
  vec3 position;
  vec3 velocity;
  quat rotation;
  vec3 rotational_velocity;
  vector<Collider> colliders;

  Body() = default;

  [[nodiscard]] mat4 getWorldTransform() const {
    mat4 s = mat4::CreateScale(scale);
    mat4 r = mat4::CreateFromQuaternion(rotation);
    mat4 t = mat4::CreateTranslation(position);

    return s * r * t;
  }
};

class SpacePartition {
  float mMinX;
  float mMaxX;
  float mMinY;
  float mMaxY;
  float mMinZ;
  float mMaxZ;
  float mStep;
  size_t mNumX;
  size_t mNumY;
  size_t mNumZ;
  vector<vector<TriangleCollider>> mPartition;

  void populate(const TriangleCollider &tc, float min_x, float max_x,
                float min_y, float max_y, float min_z, float max_z);

public:
  SpacePartition(float min_x, float max_x, float min_y, float max_y,
                 float min_z, float max_z, float step)
      : mMinX(min_x), mMaxX(max_x), mMinY(min_y), mMaxY(max_y), mMinZ(min_z),
        mMaxZ(max_z), mStep(step) {
    mNumX = ceil((max_x - min_x) / step);
    mNumY = ceil((max_y - min_y) / step);
    mNumZ = ceil((max_z - min_z) / step);

    mPartition = vector<vector<TriangleCollider>>(mNumX * mNumY * mNumZ);
  }

  SpacePartition(float min_x, float max_x, float min_y, float max_y,
                 float min_z, float max_z, float step,
                 const vector<TriangleCollider> &v)
      : mMinX(min_x), mMaxX(max_x), mMinY(min_y), mMaxY(max_y), mMinZ(min_z),
        mMaxZ(max_z), mStep(step) {
    mNumX = ceil((max_x - min_x) / step);
    mNumY = ceil((max_y - min_y) / step);
    mNumZ = ceil((max_z - min_z) / step);

    mPartition = vector<vector<TriangleCollider>>(mNumX * mNumY * mNumZ);

    populate(v);
  }

  ~SpacePartition() = default;

  void populate(const vector<TriangleCollider> &v);
  [[nodiscard]] vector<TriangleCollider> get_partition(float min_x, float max_x,
                                                       float min_y, float max_y,
                                                       float min_z,
                                                       float max_z) const;
};

// Used for spatial partitioning of static colliders
inline SpacePartition gSpacePartition =
    SpacePartition(4.0f, 150.0f, -40.0f, 80.0f, -100.0f, 10.0f, 1.0f);

namespace Physics {
bool processStaticCollisions(const vector<TriangleCollider> &triangles,
                             const SphereCollider &sphere, vec3 &velocity);

inline float physicsTicksPerSecond = 120.0;
inline float physicsDeltaTime = 1.0f / physicsTicksPerSecond;
} // namespace Physics

#endif // _PHYSICS_H
