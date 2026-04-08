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
  TriangleCollider(vec3 normal, vec3 a, vec3 b, vec3 c, vec3 position)
      : normal(normal), a(a), b(b), c(c), position(position) {}

  vec3 a;
  vec3 b;
  vec3 c;
  vec3 normal;
  vec3 position;
};

inline ostream &operator<<(ostream &os, const SphereCollider &coll) {
  os << "SphereCollider { radius: " << coll.radius
     << ", position: " << coll.position << " }";
  return os;
}

inline ostream &operator<<(ostream &os, const TriangleCollider &coll) {
  os << "TriangleCollider { a: " << coll.a << ", b: " << coll.b
     << ", c: " << coll.c << ", normal: " << coll.normal
     << ", position: " << coll.position << " }";
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
};

namespace Physics {
void Update(Body &body, float t, float dt, const vector<SphereCollider> &dc,
            SphereCollider &col, const SpacePartition &sp);
} // namespace Physics

#endif // _PHYSICS_H
