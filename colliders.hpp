#ifndef _COLLIDERS_H
#define _COLLIDERS_H

#include "maths.hpp"
#include "pch.h"

using Maths::mat4;
using Maths::quat;
using Maths::vec3;

struct Collider {};

struct SphereCollider : Collider {
  SphereCollider(vec3 &position, float radius)
      : position(position), radius(radius) {}

  vec3 position;
  float radius;

  [[nodiscard]] tuple<float, float, float, float, float, float>
  GetBounds() const {
    float minX = position.x - radius;
    float maxX = position.x + radius;
    float minY = position.y - radius;
    float maxY = position.y + radius;
    float minZ = position.z - radius;
    float maxZ = position.z + radius;

    return {minX, maxX, minY, maxY, minZ, maxZ};
  }
};

// For static objects
struct TriangleCollider : Collider {
  TriangleCollider(vec3 &normal, vec3 &a, vec3 &b, vec3 &c, GLuint vertexArray)
      : normal(normal), a(a), b(b), c(c), vertexArray(vertexArray) {}

  vec3 a;
  vec3 b;
  vec3 c;
  vec3 normal;
  // GLuint vertexBuffer;
  // GLuint indexBuffer;
  GLuint vertexArray;

  [[nodiscard]] tuple<float, float, float, float, float, float>
  GetBounds() const {
    float minX = std::min(a.x, std::min(b.x, c.x));
    float maxX = std::min(a.x, std::min(b.x, c.x));
    float minY = std::min(a.y, std::min(b.y, c.y));
    float maxY = std::max(a.y, std::max(b.y, c.y));
    float minZ = std::min(a.z, std::min(b.z, c.z));
    float maxZ = std::max(a.z, std::max(b.z, c.z));
    return {minX, maxX, minY, maxY, minZ, maxZ};
  }
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
  vec3 scale;
  vec3 position;
  quat rotation;

  Body() = default;

  [[nodiscard]] mat4 getWorldTransform() const {
    mat4 s = mat4::CreateScale(scale);
    mat4 r = mat4::CreateFromQuaternion(rotation);
    mat4 t = mat4::CreateTranslation(position);

    return s * r * t;
  }
};

struct DynamicBody : Body {
  vec3 velocity;
  vec3 rotationalVelocity;
  SphereCollider collider;

  DynamicBody(SphereCollider &c) : collider(c), Body() {}

  void RegisterInputLeft(float dt) {
    vec3 left = velocity.cross(vec3::up).normalized();
    velocity += left * 4.0f * dt;
  }
  void RegisterInputRight(float dt) {
    vec3 right = vec3::up.cross(velocity).normalized();
    velocity += right * 4.0f * dt;
  }
  void ResetToPosition(const vec3 &pos) {
    position = pos;
    velocity = vec3::zero;
    rotationalVelocity = vec3::zero;
  }
};

struct StaticBody : Body {
  StaticBody() : Body() {}
  StaticBody(Body &b) : Body(b) {}
};

#endif
