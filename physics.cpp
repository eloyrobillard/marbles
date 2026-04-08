#include "physics.h"
#include "pch.h"
#include "template.h"

// SOURCE: "Real-Time Collision Detection" by Christer Ericson
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

optional<vec3> intersectsTriangle(const TriangleCollider &t,
                                  const SphereCollider &s) {
  const vec3 &center = s.position;

  auto closest_point = ClosestPtvec3Triangle(center, t.a, t.b, t.c);

  // Sphere and triangle intersect if the (squared) distance from sphere
  // center to point p is less than the (squared) sphere radius
  vec3 v = closest_point - center;
  if (v.dot(v) > s.radius * s.radius) {
    return {};
  }

  return {(center - closest_point).normalized()};
}

optional<vec3> intersectsSphere(const SphereCollider &s1,
                                const SphereCollider &s2) {
  const vec3 &c1 = s1.position;
  const vec3 &c2 = s2.position;

  if (c1.distance(c2) > s1.radius + s2.radius)
    return {};

  // Return the direction away from c1
  // Using a unit vector is needed to compute the strength of the rebound
  return {(c1 - c2).normalized()};
}

const float restitution = 0.3f;
pair<vec3, int> processCollisions(const vector<TriangleCollider> &colls,
                                  const SphereCollider &collider, size_t start,
                                  size_t end, const vec3 &velocity) {
  vec3 collisions = vec3::zero;
  int num_collisions = 0;

  for (size_t i = start; i < end; i++) {
    const float max_x = fmax(fmax(colls[i].a.x, colls[i].b.x), colls[i].c.x);
    const float min_x = fmin(fmin(colls[i].a.x, colls[i].b.x), colls[i].c.x);

    if (abs(max_x - collider.position.x) > collider.radius &&
        abs(min_x - collider.position.x) > collider.radius)
      continue;

    auto maybe_normal = intersectsTriangle(colls[i], collider);

    if (!maybe_normal.has_value())
      continue;

    // SOURCE: "Game Physics Engine Development" by Ian Millington (section 7.2)
    const vec3 normal = maybe_normal.value();
    const float sepVel = velocity.dot(normal);
    collisions += normal * (-sepVel * (restitution + 1));
    num_collisions++;
  }

  return {collisions, num_collisions};
}

vec3 computeCollisionRebound(
    const vector<vector<TriangleCollider>> &allStaticColliders,
    const vector<SphereCollider> &allDynamicColliders, SphereCollider &collider,
    const vec3 &velocity) {
  int tot_collisions = 0;
  vec3 impulse = vec3::zero;

  for (const auto &colls : allStaticColliders) {
    auto [collisions, num_collisions] =
        processCollisions(colls, collider, 0, colls.size(), velocity);

    impulse += collisions;
    tot_collisions += num_collisions;
  }

  // for (const auto &coll : allDynamicColliders) {
  //   // Pointer comparison
  //   if (&coll != &collider) {
  //     auto maybe_collision = intersectsSphere(coll, collider);
  //
  //     if (!maybe_collision.has_value())
  //       continue;
  //
  //     collisions.emplace_back(maybe_collision.value());
  //   }
  // }

  if (tot_collisions > 0)
    impulse *= (1.0f / static_cast<float>(tot_collisions));
  return impulse;
}

void Physics::Update(Body &body, float t, float dt,
                     const vector<vector<TriangleCollider>> &sc,
                     const vector<SphereCollider> &dc, SphereCollider &col) {
  const vec3 prev_v = body.velocity;
  const vec3 prev_p = body.position;

  body.velocity += dt / 1024.0f * grav_force;
  body.position += dt / 1024.0f * body.velocity;

  vec3 rebound = computeCollisionRebound(sc, dc, col, prev_v);

  body.velocity = prev_v + rebound + dt / 1024.0f * grav_force;
  body.position = prev_p + dt / 1024.0f * body.velocity;

  col.position = body.position;
}
