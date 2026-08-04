#ifndef _COLLIDERS_H
#define _COLLIDERS_H

#include "maths.hpp"
#include "pch.h"

using Maths::mat4;
using Maths::quat;
using Maths::vec3;
using Maths::vec4;

class Body;
class StaticBody;

struct AngleBounds {
  float minRad;
  float maxRad;
  // この二つは四元数のスカラ部と直接比較するために役立つ
  float maxSinHalfRad;
  float minSinHalfRad;
};

struct Collider {};

struct SphereCollider : Collider {
  SphereCollider(vec3 &position, float radius)
      : position(position), radius(radius) {}

  vec3 position;
  float radius;
};

template <class T>
concept HasRotation = requires(T t) {
  { t.rotation } -> std::convertible_to<Maths::quat>;
};

// For (mostly) static objects
template <HasRotation T>
struct TriangleCollider : Collider {
  TriangleCollider(vec3 &normal, vec3 &a, vec3 &b, vec3 &c,
                   const shared_ptr<T> &body, GLuint vertexBuffer,
                   GLuint indexBuffer, GLuint vertexArray)
      : initNormal(normal), normal(normal), a(a), b(b), c(c), body(body),
        vertexBuffer(vertexBuffer), indexBuffer(indexBuffer),
        vertexArray(vertexArray) {}

  void updateNormal() {
    normal = quat::RotateVector(body->rotation, initNormal);
    normal.normalize();
  }

  vec3 a;
  vec3 b;
  vec3 c;
  vec3 normal;
  vec3 initNormal;
  const shared_ptr<T> body;
  const GLuint vertexBuffer;
  const GLuint indexBuffer;
  const GLuint vertexArray;
};

inline ostream &operator<<(ostream &os, const SphereCollider &coll) {
  os << "SphereCollider { radius: " << coll.radius
     << ", position: " << coll.position << " }";
  return os;
}

template <HasRotation T>
inline ostream &operator<<(ostream &os, const TriangleCollider<T> &coll) {
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
  vec3 rotationAxis;
  SphereCollider collider;

  DynamicBody(SphereCollider &c) : Body(), collider(c) {}

  void RegisterInputForward(float dt, const vec3 &cameraForward) {
    velocity += cameraForward * 4.0f * dt;
    rotationalVelocity = velocity;
  }
  void RegisterInputBackward(float dt, const vec3 &cameraForward) {
    velocity -= cameraForward * 4.0f * dt;
    rotationalVelocity = velocity;
  }
  void RegisterInputLeft(float dt, const vec3 &cameraRight) {
    velocity -= cameraRight * 4.0f * dt;
    rotationalVelocity = velocity;
  }
  void RegisterInputRight(float dt, const vec3 &cameraRight) {
    velocity += cameraRight * 4.0f * dt;
    rotationalVelocity = velocity;
  }
  void ResetToPosition(const vec3 &pos) {
    position = pos;
    velocity = vec3::zero;
    rotationalVelocity = vec3::zero;
    rotation = quat::Identity;
  }
};

struct PivotBody : Body {
  PivotBody() : Body() {}
  PivotBody(Body &b, float accel, bool oi, bool os, vec3 io, vec3 so)
      : Body(b), collisionAcceleration(accel), overrideImpulse(oi),
        overrideSpeed(os), impulseOverride(io), speedOverride(so) {}

  PivotBody(Body &b, const vec3 &pivotAxis, const vec3 &pivotPoint,
            pair<float, float> boundsDeg, float resistance)
      : Body(b), pivotAxis(pivotAxis), pivotPoint(pivotPoint),
        constantAcceleration(quat(pivotAxis, resistance)) {
    angleBounds.minRad = boundsDeg.first / 180.0f * Maths::PI;
    angleBounds.maxRad = boundsDeg.second / 180.0f * Maths::PI;
    // NOTE: これで回転範囲計算においてフレームごとに角度を計算せずに済む
    angleBounds.minSinHalfRad = sinf(angleBounds.minRad / 2.0f);
    angleBounds.maxSinHalfRad = sinf(angleBounds.maxRad / 2.0f);
  }

  quat rotationalVelocity;
  float collisionAcceleration = 1.0f;
  bool overrideImpulse = false;
  bool overrideSpeed = false;
  vec3 impulseOverride = vec3::zero;
  vec3 speedOverride = vec3::zero;
  vec3 pivotAxis = vec3::zero;
  vec3 pivotPoint = vec3::zero;
  AngleBounds angleBounds = {0.0f, 45.0f};
  quat constantAcceleration;
};

struct StaticBody : Body {
  StaticBody() : Body() {}
  StaticBody(Body &b, float accel, bool oi, bool os, vec3 io, vec3 so)
      : Body(b), collisionAcceleration(accel), overrideImpulse(oi),
        overrideSpeed(os), impulseOverride(io), speedOverride(so) {}

  float collisionAcceleration = 1.0f;
  bool overrideImpulse = false;
  bool overrideSpeed = false;
  vec3 impulseOverride = vec3::zero;
  vec3 speedOverride = vec3::zero;
};

#endif
