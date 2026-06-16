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
};

// For static objects
struct TriangleCollider : Collider {
  TriangleCollider(vec3 &normal, vec3 &a, vec3 &b, vec3 &c, float accel,
                   bool overrideSpeed, vec3 &speedOverride,
                   bool overrideImpulse, vec3 &impulseOverride,
                   GLuint vertexBuffer, GLuint indexBuffer, GLuint vertexArray)
      : normal(normal), a(a), b(b), c(c), accel(accel),
        overrideSpeed(overrideSpeed), speedOverride(speedOverride),
        overrideImpulse(overrideImpulse), impulseOverride(impulseOverride),
        vertexBuffer(vertexBuffer), indexBuffer(indexBuffer),
        vertexArray(vertexArray) {}

  vec3 a;
  vec3 b;
  vec3 c;
  float accel;
  bool overrideSpeed;
  vec3 speedOverride;
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
