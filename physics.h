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
  SphereCollider(vec3 *position, float radius)
      : position(position), radius(radius) {}

  float radius;
  vec3 *position;
};

// For static objects
struct TriangleCollider : Collider {
  TriangleCollider(vec3 normal, vec3 a, vec3 b, vec3 c, vec3 *position)
      : normal(normal), a(a), b(b), c(c), position(position) {}

  vec3 a;
  vec3 b;
  vec3 c;
  vec3 normal;
  vec3 *position;
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
      : scale(vec3()), position(vec3()), velocity(vec3()), rotation(quat()),
        rotational_velocity(vec3()), colliders({}) {}
  vec3 scale;
  vec3 position;
  vec3 velocity;
  quat rotation;
  vec3 rotational_velocity;
  vector<Collider> colliders;
};

namespace Physics {
void Update(Body &body, float t, float dt, vector<vector<TriangleCollider>> &sc,
            vector<SphereCollider> &dc, SphereCollider &col);
} // namespace Physics

#endif // _PHYSICS_H
