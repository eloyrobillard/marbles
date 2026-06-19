#include "physics.hpp"
#include "entities.hpp"
#include "pch.h"

// SOURCE: "Real-Time Collision Detection" by Christer Ericson (5.1.5 & 5.2.7)
vec3 ClosestPtvec3Triangle(const vec3 &p, const vec3 &a, const vec3 &b,
                           const vec3 &c) {
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

optional<vec3> intersectsTriangle(const TriangleCollider &t,
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

  return {v};
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
      eThis.velocity += normal * (-sepVel * (restitution + 1));
      eOther.velocity += -normal * (-sepVelOther * (restitution + 1));
    }
  }

  return -1;
}

bool Physics::processStaticCollisions(const vector<TriangleCollider> &triangles,
                                      const Mesh &mesh,
                                      const SphereCollider &sphere,
                                      vec3 &velocity) {
  bool collision_happened = false;

  for (const auto &triangle : triangles) {
    auto maybe_normal = intersectsTriangle(triangle, sphere);

    if (!maybe_normal.has_value())
      continue;

    collision_happened = true;

#ifdef _DEBUG
    gToRenderAsCollided.push(triangle.vertexArray);
#endif

    // SOURCE: "Game Physics Engine Development" by Ian Millington
    // (section 7.2)
    vec3 normal = triangle.normal.normalized();
    if (normal.dot(maybe_normal.value()) > 0) {
      normal *= -1.0f;
    }

    const float sepVel = velocity.dot(normal);

    if (sepVel < 0) {
      if (mesh.overrideImpulse) {
        velocity = mesh.impulseOverride * velocity.length() * mesh.accel;
      } else if (mesh.overrideSpeed) {
        velocity = mesh.speedOverride;
      } else {
        // Apply impulse instantly
        velocity += normal * (-sepVel * (restitution + 1)) * mesh.accel;
      }
    }
  }

  return collision_happened;
}
