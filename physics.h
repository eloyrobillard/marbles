#ifndef _PHYSICS_H
#define _PHYSICS_H
#pragma once

#include "pch.h"
#include "template.h"

using Tmpl8::quat;
using Tmpl8::vec3;

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
  TriangleCollider(vec3 normal, vec3 a, vec3 b, vec3 c, GLuint vertexBuffer,
                   GLuint indexBuffer, GLuint vertexArray)
      : normal(normal), a(a), b(b), c(c), vertexBuffer(vertexBuffer),
        indexBuffer(indexBuffer), vertexArray(vertexArray) {}

  vec3 a;
  vec3 b;
  vec3 c;
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

struct Body {
  Body()
      : scale(vec3(1.0f)), position(vec3()), velocity(vec3()), rotation(quat()),
        rotational_velocity(vec3()), colliders({}) {}
  vec3 scale;
  vec3 position;
  vec3 velocity;
  quat rotation;
  vec3 rotational_velocity;
  vector<Collider> colliders;
};

class SpacePartition {
public:
  float mMax_x;
  float mMin_x;

  SpacePartition(float min_x, float max_x) : mMin_x(min_x), mMax_x(max_x) {}
  virtual ~SpacePartition() = default;

  virtual void populate(const TriangleCollider &tc, float min_x,
                        float max_x) = 0;
  virtual void populate(const vector<TriangleCollider> &v) = 0;
  [[nodiscard]] virtual vector<TriangleCollider>
  get_partition(const SphereCollider &s, float min_x, float max_x) const = 0;

  virtual void print(ostream &os) const = 0;

  friend ostream &operator<<(ostream &os, const SpacePartition &n) {
    n.print(os);
    return os;
  }
};

class SPNode : public SpacePartition {
  vector<unique_ptr<SpacePartition>> mChildren;

public:
  using SpacePartition::SpacePartition;
  SPNode(float min_x, float max_x, int depth, int num_children);
  ~SPNode() override = default;

  void populate(const TriangleCollider &tc, float min_x, float max_x) override;
  void populate(const vector<TriangleCollider> &v) override;
  [[nodiscard]] vector<TriangleCollider>
  get_partition(const SphereCollider &s, float min_x,
                float max_x) const override;
  void print(ostream &os) const override;
};

class SPLeaf : public SpacePartition {
  vector<TriangleCollider> mPartition;

public:
  using SpacePartition::SpacePartition;
  SPLeaf(float min_x, float max_x)
      : mPartition(vector<TriangleCollider>()),
        SpacePartition::SpacePartition(min_x, max_x) {}
  ~SPLeaf() override = default;

  void populate(const vector<TriangleCollider> &v) override {
    mPartition.append_range(v);
  };
  void populate(const TriangleCollider &tc, float min_x, float max_x) override {
    mPartition.emplace_back(tc);
  }
  [[nodiscard]] vector<TriangleCollider>
  get_partition(const SphereCollider &s, float min_x,
                float max_x) const override {
    return mPartition;
  };
  void print(ostream &os) const override;
};

// Used for spatial partitioning of static colliders
inline SPNode gSP = SPNode(5.0f, 25.0f, 1, 16);

namespace Physics {
bool computeCollisionRebound(const SpacePartition &sp,
                             const SphereCollider &collider, vec3 &velocity);

inline float physicsTicksPerSecond = 60.0;
inline float physicsDeltaTime = 1.0f / physicsTicksPerSecond;
} // namespace Physics

#endif // _PHYSICS_H
