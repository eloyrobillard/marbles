#include "physics.hpp"
#include "entities.hpp"
#include "pch.h"

void SpacePartition::populate(const vector<TriangleCollider> &v) {
  for (const auto &tc : v) {
    float min_x = fmin(fmin(tc.a.x, tc.b.x), tc.c.x);
    float max_x = fmax(fmax(tc.a.x, tc.b.x), tc.c.x);
    float min_y = fmin(fmin(tc.a.y, tc.b.y), tc.c.y);
    float max_y = fmax(fmax(tc.a.y, tc.b.y), tc.c.y);
    float min_z = fmin(fmin(tc.a.z, tc.b.z), tc.c.z);
    float max_z = fmax(fmax(tc.a.z, tc.b.z), tc.c.z);

    auto start_x =
        std::max((size_t)0, static_cast<size_t>((min_x - mMinX) / mStep));
    auto end_x =
        std::min(mNumX - 1, static_cast<size_t>((max_x - mMinX) / mStep));
    auto start_y =
        std::max((size_t)0, static_cast<size_t>((min_y - mMinY) / mStep));
    auto end_y =
        std::min(mNumY - 1, static_cast<size_t>((max_y - mMinY) / mStep));
    auto start_z =
        std::max((size_t)0, static_cast<size_t>((min_z - mMinZ) / mStep));
    auto end_z =
        std::min(mNumZ - 1, static_cast<size_t>((max_z - mMinZ) / mStep));

    for (size_t x = start_x; x <= end_x; x++) {
      for (size_t y = start_y; y <= end_y; y++) {
        for (size_t z = start_z; z <= end_z; z++) {
          mPartition[x * mNumY * mNumZ + y * mNumZ + z].emplace_back(tc);
        }
      }
    }
  }
}

vector<TriangleCollider> SpacePartition::get_partition(float min_x, float max_x,
                                                       float min_y, float max_y,
                                                       float min_z,
                                                       float max_z) const {
  vector<TriangleCollider> result{};
  auto start_x =
      std::max((size_t)0, static_cast<size_t>((min_x - mMinX) / mStep));
  size_t end_x =
      std::min(mNumX - 1, static_cast<size_t>((max_x - mMinX) / mStep));
  auto start_y =
      std::max((size_t)0, static_cast<size_t>((min_y - mMinY) / mStep));
  size_t end_y =
      std::min(mNumY - 1, static_cast<size_t>((max_y - mMinY) / mStep));
  auto start_z =
      std::max((size_t)0, static_cast<size_t>((min_z - mMinZ) / mStep));
  size_t end_z =
      std::min(mNumZ - 1, static_cast<size_t>((max_z - mMinZ) / mStep));

  for (size_t x = start_x; x <= end_x; x++) {
    for (size_t y = start_y; y <= end_y; y++) {
      for (size_t z = start_z; z <= end_z; z++) {
        result.append_range(mPartition[x * mNumY * mNumZ + y * mNumZ + z]);
      }
    }
  }

  return result;
};

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

  vec3 normal = t.normal.normalized();

  if (t.normal.dot(v) > 0) {
    return {-normal};
  }

  return {normal};
}

const float restitution = 0.0f;
bool processCollisions(const vector<TriangleCollider> &triangles,
                       const SphereCollider &sphere, vec3 &velocity) {
  vec3 collisions = vec3::zero;
  int num_collisions = 0;
  bool collision_happened = false;

  for (const auto &triangle : triangles) {
    auto maybe_normal = intersectsTriangle(triangle, sphere);

    if (!maybe_normal.has_value())
      continue;

    collision_happened = true;

#ifdef _DEBUG
    gTo_render_as_collided.push(triangle.vertexArray);
#endif

    // SOURCE: "Game Physics Engine Development" by Ian Millington (section 7.2)
    const vec3 normal = maybe_normal.value();
    const float sepVel = velocity.dot(normal);

    if (sepVel < 0) {
      // Apply impulse instantly
      if (triangle.overrideImpulse) {
        velocity =
            triangle.impulseOverride * velocity.length() * triangle.accel;
      } else {
        velocity += normal * (-sepVel * (restitution + 1)) * triangle.accel;
      }
    }
  }

  return collision_happened;
}

bool Physics::getCollisionImpulse(const SpacePartition &sp,
                                  const SphereCollider &collider,
                                  vec3 &velocity) {
  float min_x = collider.position.x - collider.radius;
  float max_x = collider.position.x + collider.radius;
  float min_y = collider.position.y - collider.radius;
  float max_y = collider.position.y + collider.radius;
  float min_z = collider.position.z - collider.radius;
  float max_z = collider.position.z + collider.radius;

  gCurrent_partition =
      sp.get_partition(min_x, max_x, min_y, max_y, min_z, max_z);

  return processCollisions(gCurrent_partition, collider, velocity);
}
