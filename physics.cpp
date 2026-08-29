#include "physics.hpp"
#include "audio.hpp"
#include "entities.hpp"
#include "pch.h"

// SOURCE: "Real-Time Collision Detection" by Christer Ericson (5.1.5 & 5.2.7)
vec3 ClosestPtvec3Triangle(vec3 p, vec3 a, vec3 b, vec3 c) {
  vec3 ab = b - a;
  vec3 ac = c - a;
  vec3 bc = c - b;
  vec3 ap = p - a;
  vec3 bp = p - b;
  vec3 cp = p - c;
  vec3 ba = -ab;
  vec3 ca = -ac;
  vec3 cb = -bc;
  vec3 pa = -ap;
  vec3 pb = -bp;
  vec3 pc = -cp;

  // Compute parametric position s for projection P’ of P on AB,
  // P’ = A + s*AB, s = snom/(snom+sdenom)
  float snom = ap.dot(ab), sdenom = bp.dot(ba);

  // Compute parametric position t for projection P’ of P on AC,
  // P’ = A + t*AC, s = tnom/(tnom+tdenom)
  float tnom = ap.dot(ac), tdenom = cp.dot(ca);
  if (snom <= 0.0f && tnom <= 0.0f)
    return a; // Vertex region early out

  // Compute parametric position u for projection P’ of P on BC,
  // P’ = B + u*BC, u = unom/(unom+udenom)
  float unom = bp.dot(bc), udenom = cp.dot(cb);
  if (sdenom <= 0.0f && unom <= 0.0f)
    return b; // Vertex region early out
  if (tdenom <= 0.0f && udenom <= 0.0f)
    return c; // Vertex region early out

  // P is outside (or on) AB if the triple scalar product [N PA PB] <= 0
  vec3 n = (ab).cross(ac);
  float vc = n.dot((pa).cross(pb));

  // If P outside AB and within feature region of AB,
  // return projection of P onto AB
  if (vc <= 0.0f && snom >= 0.0f && sdenom >= 0.0f)
    return a + snom / (snom + sdenom) * ab;

  // P is outside (or on) BC if the triple scalar product [N PB PC] <= 0
  float va = n.dot((pb).cross(pc));
  // If P outside BC and within feature region of BC,
  // return projection of P onto BC
  if (va <= 0.0f && unom >= 0.0f && udenom >= 0.0f)
    return b + unom / (unom + udenom) * bc;

  // P is outside (or on) CA if the triple scalar product [N PC PA] <= 0
  float vb = n.dot((pc).cross(pa));
  // If P outside CA and within feature region of CA,
  // return projection of P onto CA
  if (vb <= 0.0f && tnom >= 0.0f && tdenom >= 0.0f)
    return a + tnom / (tnom + tdenom) * ac;

  // P must project inside face region. Compute Q using barycentric coordinates
  float u = va / (va + vb + vc);
  float v = vb / (va + vb + vc);
  float w = 1.0f - u - v; // = vc / (va + vb + vc)
  return u * a + v * b + w * c;
}

struct CollisionData {
  vec3 normal;
  vec3 collisionPoint;
};

bool Physics::segmentAndTriangleIntersect(const vec3 &P, const vec3 &Q,
                                          const vec3 &A, const vec3 &B,
                                          const vec3 &C) {
  const vec3 QP = P - Q;
  const vec3 AC = C - A;
  const vec3 AB = B - A;

  const vec3 n = AB.cross(AC);

  const f32 d = QP.dot(n);
  if (d <= 0.f)
    return false;

  const vec3 AP = P - A;
  const f32 t = AP.dot(n);
  if (t < 0.f || t > d)
    return false;

  const vec3 e = QP.cross(AP);
  const f32 v = AC.dot(e);
  if (v < 0.f || v > d)
    return false;
  const f32 w = -AB.dot(e);
  if (w < 0.f || v + w > d)
    return false;

  return true;
}

template <class T>
  requires std::derived_from<T, Body>
optional<CollisionData> intersectsTriangle(const TriangleCollider<T> &t,
                                           const SphereCollider &s) {
  const vec3 &center = s.position;

  auto closest_point = ClosestPtvec3Triangle(center, t.a, t.b, t.c);

  // Sphere and triangle intersect if the (squared) distance from sphere
  // center to point p is less than the (squared) sphere radius
  vec3 v = closest_point - center;
  float v2 = v.dot(v);
  if (isnan(v2) || v2 > s.radius * s.radius) {
    return {};
  }

  if (t.normal.dot(v) > 0) {
    return {{-t.normal, closest_point}};
  }

  return {{t.normal, closest_point}};
}

template <class T>
  requires std::derived_from<T, Body>
optional<CollisionData> intersectsTriangle(const TriangleCollider<T> *t,
                                           const SphereCollider &s) {
  const vec3 &center = s.position;

  auto closest_point = ClosestPtvec3Triangle(center, t->a, t->b, t->c);

  // Sphere and triangle intersect if the (squared) distance from sphere
  // center to point p is less than the (squared) sphere radius
  vec3 v = closest_point - center;
  float v2 = v.dot(v);
  if (isnan(v2) || v2 > s.radius * s.radius) {
    return {};
  }

  if (t->normal.dot(v) > 0) {
    return {{-t->normal, closest_point}};
  }

  return {{t->normal, closest_point}};
}

optional<vec3> intersectsSphere(SphereCollider &s1, SphereCollider &s2) {
  const vec3 diff = s1.position - s2.position;
  if (diff.length() > s1.radius + s2.radius) {
    return {};
  } else {
    return {diff.normalized()};
  }
}

const float restitution = 0.0f;
int Physics::processDynamicCollisions(vector<DynamicBody> &des, int idx,
                                      int numMarbles, bool joining) {
  DynamicBody eThis = des[idx];

  // Check for collision against spheres past this one in the vector
  // This is to avoid duplicate collision checks
  for (int i = idx + 1; i < numMarbles; i++) {
    DynamicBody &eOther = des[i];
    auto maybeNormal = intersectsSphere(eThis.collider, eOther.collider);

    if (!maybeNormal.has_value())
      continue;

    if (joining) {
      return i;
    }

    // SOURCE: "Game Physics Engine Development" by Ian Millington
    // (section 7.2)
    const vec3 normal = maybeNormal.value();
    const float sepVel = eThis.velocity.dot(normal);
    const float sepVelOther = eOther.velocity.dot(-normal);

    // Apply impulse instantly
    if (sepVel < 0 || sepVelOther < 0) {
      AudioMachine::PlayCollision(-sepVel);

      eThis.velocity += normal * (-sepVel * (restitution + 1));
      eOther.velocity += -normal * (-sepVelOther * (restitution + 1));
      eThis.rotationalVelocity = eThis.velocity;
      eOther.rotationalVelocity = eOther.velocity;
    }
  }

  return -1;
}

// TODO: より汎用的な関数に変換：法線、軸などだけもらって、回転を返すやつ
quat computeDoorRotation(const TriangleCollider<PivotBody> &triangle,
                         const float sepVel, const float distToPivot) {
  // 速度(sepVel)から角速度を導出
  float angVel = 20.f * sepVel / distToPivot;

  quat rotVel(triangle.body.pivotAxis, angVel);

  return quat::Concatenate(triangle.body.rotationalVelocity, rotVel);
}

bool Physics::processDoorCollisions(
    const vector<pair<TriangleCollider<PivotBody>, mat4>> &triangles,
    DynamicBody &marble) {
  bool collision_happened = false;

  for (const auto &[triangle, model] : triangles) {
    auto maybe_coll = intersectsTriangle(triangle, marble.collider);

    if (!maybe_coll.has_value())
      continue;

    collision_happened = true;

#ifdef _DEBUG
    gRenderDoorAsCollided.emplace_back(triangle.vertexArray, model);
#endif

    // SOURCE: "Game Physics Engine Development" by Ian Millington
    // (section 7.2)
    const auto &[normal, closestPoint] = maybe_coll.value();
    const float sepVel = marble.velocity.dot(normal);

    if (sepVel < 0) {
      // Apply impulse instantly
      if (triangle.body.overrideImpulse) {
        marble.velocity = triangle.body.impulseOverride *
                          marble.velocity.length() *
                          triangle.body.collisionAcceleration;
      } else if (triangle.body.overrideSpeed) {
        marble.velocity = triangle.body.speedOverride;
      } else {
        marble.velocity += normal * (-sepVel * (restitution + 1)) *
                           triangle.body.collisionAcceleration;
      }

      marble.rotationalVelocity = marble.velocity;

      vec3 axis = triangle.body.pivotAxis;
      vec3 point = triangle.body.pivotPoint;
      float t =
          (closestPoint.x + closestPoint.y + closestPoint.z - axis.dot(point)) /
          axis.sqrLentgh();
      vec3 pointOnPivotAxis = point + t * axis;

      triangle.body.rotationalVelocity = computeDoorRotation(
          triangle, sepVel, closestPoint.distance(pointOnPivotAxis));
    }
  }

  return collision_happened;
}

bool Physics::processStaticCollisions(
    const vector<TriangleCollider<StaticBody> *> &triangles,
    DynamicBody &marble) {
  bool onGround = false;

  for (const auto triangle : triangles) {
    auto maybe_coll = intersectsTriangle(triangle, marble.collider);

    if (!maybe_coll.has_value()) {
      auto maybe_onground = intersectsTriangle(
          triangle,
          SphereCollider(marble.position, marble.collider.radius * 4.f));

      if (maybe_onground.has_value())
        onGround = true;

      continue;
    }

#ifdef _DEBUG
    gRenderAsCollided.push_back(triangle->vertexArray);
#endif

    // SOURCE: "Game Physics Engine Development" by Ian Millington
    // (section 7.2)
    const auto &[normal, closestPoint] = maybe_coll.value();
    const float sepVel = marble.velocity.dot(normal);

    if (sepVel < 0) {
      AudioMachine::PlayCollision(-sepVel);

      // Apply impulse instantly
      if (triangle->body.overrideImpulse) {
        marble.velocity = triangle->body.impulseOverride *
                          marble.velocity.length() *
                          triangle->body.collisionAcceleration;
      } else if (triangle->body.overrideSpeed) {
        marble.velocity = triangle->body.speedOverride;
      } else {
        marble.velocity += normal * (-sepVel * (restitution + 1)) *
                           triangle->body.collisionAcceleration;
      }

      marble.rotationalVelocity = marble.velocity;
    }
  }

  return onGround;
}

bool Physics::Raycast(const Maths::Ray &ray, f32 startDistance,
                      f32 maxDistance) {
  vec3 unitToTarget = ray.direction.normalized();

  float t = startDistance;
  while (t <= maxDistance) {
    const vec3 P = ray.origin + t * unitToTarget;
    const vec3 Q = ray.origin + (t + 0.25f) * unitToTarget;

    const auto staticColliders =
        gSpacePartition.getPartition(Q.x, Q.x, Q.y, Q.y, Q.z, Q.z);

    for (const auto &triangle : staticColliders) {
      if (Physics::segmentAndTriangleIntersect(Q, P, triangle->a, triangle->b,
                                               triangle->c)) {
        return true;
      }
    }

    t += 0.25f;
  }

  return false;
}
