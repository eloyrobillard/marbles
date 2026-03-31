#include "physics.h"
#include "pch.h"
#include "template.h"

pair<vec3, vec3> getClosestPointOnTriangle(const vec3 point, const vec3 &normal,
                                           const vec3 &a, const vec3 &b,
                                           const vec3 &c) {
  // To see if our projection is in the triangle, we first get the closest point
  // belonging to an edge of the triangle
  vec3 closest_point{std::numeric_limits<float>::infinity()};
  int closest_idx = -1;

  // Combinations of opposite point (idx 0) to edge (idx 1 & 2)
  vec3 combs[3][3] = {{a, b, c}, {b, c, a}, {c, a, b}};

  for (int i = 0; i < 3; i++) {
    vec3 opposite = combs[i][0];

    // Arbitrarily naming the edge "AB"
    vec3 A = combs[i][1];
    vec3 B = combs[i][2];

    // Direction vector for line containing edge
    vec3 u = B - A;
    // Vector belonging to same plane as triangle and perpendicular to AB
    vec3 v = u.cross(normal);

    // Find by what factor v needs to be multiplied to get the closest point the
    // projected center (C')
    float t_orth = (A.x * u.y - point.x * u.y + u.x * point.y - A.y * u.x) /
                   (v.x * u.y - u.x * v.y);

    vec3 intersection_point = point + t_orth * v;

    bool is_on_triangle = 0 <= t_orth && t_orth <= 1;
    bool is_closer =
        point.distance(intersection_point) < point.distance(closest_point);

    if (is_on_triangle && is_closer) {
      closest_point = intersection_point;
      closest_idx = i;
    }
  }

  return {closest_point, combs[closest_idx][0]};
}

// Derive the coefficients of the triangle's plane from its normal
// and one vertex: ax + by + cz + d = 0
tuple<float, float, float, float>
getPlaneFromNormalAndPoint(const vec3 &normal, const vec3 &point) {
  float a = normal.x;
  float b = normal.y;
  float c = normal.z;
  float d = -(a * point.x + b * point.y + c * point.z);

  return {a, b, c, d};
}

optional<vec3> intersectsTriangle(const TriangleCollider &t,
                                  const SphereCollider &s) {
  const vec3 &center = s.position;

  auto [a, b, c, d] = getPlaneFromNormalAndPoint(t.normal, t.a);

  // Get the sphere center's projection on the triangle's plane
  float lambda = -(a * center.x + b * center.y + c * center.z + d) /
                 t.normal.dot(t.normal);
  vec3 projected_center = center + t.normal * lambda;

  // If the sphere doesn't intersect the triangle's plane, no need to look
  // further
  bool sphere_intersects_plane = projected_center.distance(center) <= s.radius;
  if (!sphere_intersects_plane) {
    return {};
  }

  auto [closest_point, opposite] =
      getClosestPointOnTriangle(projected_center, t.normal, t.a, t.b, t.c);

  // If we find our projected center to be in the triangle
  // that's definitely the closest point to the sphere's center
  bool projection_is_in_triangle =
      opposite.distance(closest_point) > opposite.distance(projected_center);
  if (projection_is_in_triangle) {
    closest_point = projected_center;
  }

  // If the closest point on the triangle is not within the sphere's radius,
  // no collision
  if (closest_point.distance(center) > s.radius) {
    return {};
  }

  // Return the normal of the triangle pointing towards the sphere
  const bool hit_backside_of_triangle =
      (center - closest_point).dot(t.normal) < 0;
  const vec3 outward_normal = hit_backside_of_triangle ? -t.normal : t.normal;

  // Using a unit vector is needed to compute the strength of the rebound
  return {outward_normal.normalized()};
}

optional<vec3> intersectsSphere(const SphereCollider &s1,
                                const SphereCollider &s2) {
  const vec3 &c1 = s1.position;
  const vec3 &c2 = s2.position;

  if (c1.distance(c2) > s1.radius + s2.radius)
    return {};

  // Return the direction away from c1
  // Using a unit vector is needed to compute the strength of the rebound
  return {(c2 - c1).normalized()};
}

void processCollisions(vector<vec3> &collisions,
                       const vector<TriangleCollider> &colls,
                       const SphereCollider &collider, size_t start,
                       size_t end) {
  for (size_t i = start; i < end; i++) {
    auto maybe_collision = intersectsTriangle(colls[i], collider);

    if (!maybe_collision.has_value())
      continue;

    collisions.emplace_back(maybe_collision.value());
  }
}

vector<vec3>
getCollisionNormals(const vector<vector<TriangleCollider>> &allStaticColliders,
                    const vector<SphereCollider> &allDynamicColliders,
                    SphereCollider &collider) {
  vector<vec3> collisions{};

  for (const auto &colls : allStaticColliders) {
    thread t1([&]() {
      processCollisions(collisions, colls, collider, 0, colls.size() / 2);
    });
    thread t2([&]() {
      processCollisions(collisions, colls, collider, colls.size() / 2,
                        colls.size());
    });

    // for (const auto &coll : colls) {
    //   auto maybe_collision = intersectsTriangle(coll, collider);
    //
    //   if (!maybe_collision.has_value())
    //     continue;
    //
    //   collisions.emplace_back(maybe_collision.value());
    // }

    t1.join();
    t2.join();
  }

  for (const auto coll : allDynamicColliders) {
    // Pointer comparison
    if (coll.position != collider.position) {
      auto maybe_collision = intersectsSphere(coll, collider);

      if (!maybe_collision.has_value())
        continue;

      collisions.emplace_back(maybe_collision.value());
    }
  }

  return collisions;
}

void Physics::Update(Body &body, float t, float dt,
                     const vector<vector<TriangleCollider>> &sc,
                     const vector<SphereCollider> &dc, SphereCollider &col) {
  const vec3 prev_v = body.velocity;
  const vec3 prev_p = body.position;

  body.velocity += dt / 1024.0f * grav_force;
  body.position += dt / 1024.0f * body.velocity;

  vector<vec3> normals = getCollisionNormals(sc, dc, col);

  // NOTE: Reset the sphere to its position before the collisions happened
  // This is to avoid colliding many times with the same object
  // before getting pushed out
  // Compute the new direction (velocity)
  auto forces = normals | transform([&](const vec3 normal) {
                  return normal * body.velocity;
                });

  body.velocity = std::accumulate(ALL(forces), body.velocity);
  body.position = prev_p + dt / 1024.0f * body.velocity;

  col.position = body.position;

  std::println(stdout, "{} {} {}, collisions: {}", body.position.x,
               body.position.y, body.position.z, normals.size());
}
