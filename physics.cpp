#include "physics.h"
#include "pch.h"
#include "template.h"

void SPNode::print(ostream &os) const {
  os << "{\n\tFrom " << mMin_x << " to " << mMax_x << ": ";
  for (const auto &child : mChildren) {
    child->print(os);
    os << ", ";
  }
  os << "\n}";
}

void SPLeaf::print(ostream &os) const {
  os << "{\n\tFrom " << mMin_x << " to " << mMax_x << ": ";
  for (const auto &coll : mPartition)
    os << coll << ", ";
  os << "\n}";
}

SPNode::SPNode(float min_x, float max_x, int depth, int num_children)
    : mChildren(vector<unique_ptr<SpacePartition>>()),
      SpacePartition::SpacePartition(min_x, max_x) {
  float step = (max_x - min_x) / static_cast<float>(num_children);

  if (depth > 1) {
    for (float i = min_x; i < max_x; i += step) {
      mChildren.emplace_back(
          new SPNode(i, i + step - 0.0001, depth - 1, num_children));
    }
  }

  for (float i = min_x; i < max_x; i += step) {
    mChildren.emplace_back(new SPLeaf(i, i + step - 0.0001));
  }
}

void SPNode::populate(const vector<TriangleCollider> &v) {
  for (const auto &tc : v) {
    float min_x = fmin(fmin(tc.a.x, tc.b.x), tc.c.x);
    float max_x = fmax(fmax(tc.a.x, tc.b.x), tc.c.x);

    populate(tc, min_x, max_x);
  }
}

void SPNode::populate(const TriangleCollider &tc, float min_x, float max_x) {
  for (auto &child : mChildren) {
    if (max_x < child->mMin_x || min_x > child->mMax_x)
      continue;

    child->populate(tc, min_x, max_x);
  }
}

vector<TriangleCollider> SPNode::get_partition(const SphereCollider &s,
                                               float min_x, float max_x) const {
  vector<TriangleCollider> result{};
  for (const auto &child : mChildren) {
    if (max_x <= child->mMin_x || min_x >= child->mMax_x)
      continue;

    result.append_range(child->get_partition(s, min_x, max_x));
  }

  return result;
};

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

  return {t.normal.normalized()};
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
bool processCollisions(const vector<TriangleCollider> &triangles,
                       const SphereCollider &sphere, vec3 &velocity) {
  vec3 collisions = vec3::zero;
  int num_collisions = 0;
  bool collision_happened = false;

  for (const auto &triangle : triangles) {
    const float max_x = fmax(fmax(triangle.a.x, triangle.b.x), triangle.c.x);
    const float min_x = fmin(fmin(triangle.a.x, triangle.b.x), triangle.c.x);

    if (abs(max_x - sphere.position.x) > sphere.radius &&
        abs(min_x - sphere.position.x) > sphere.radius)
      continue;

    auto maybe_normal = intersectsTriangle(triangle, sphere);

    if (!maybe_normal.has_value())
      continue;

    collision_happened = true;

#ifdef _DEBUG
    to_render_as_collided.push(triangle.vertexArray);
#endif

    // SOURCE: "Game Physics Engine Development" by Ian Millington (section 7.2)
    const vec3 normal = maybe_normal.value();
    const float sepVel = velocity.dot(normal);

    if (sepVel < 0) {
      // Apply impulse instantly
      velocity += normal * (-sepVel * (restitution + 1));
    }
  }

  return collision_happened;
}

bool computeCollisionRebound(const SpacePartition &sp,
                             const vector<SphereCollider> &allDynamicColliders,
                             const SphereCollider &collider, vec3 &velocity) {
  float min_x = collider.position.x - collider.radius;
  float max_x = collider.position.x + collider.radius;

  current_partition = sp.get_partition(collider, min_x, max_x);

  return processCollisions(current_partition, collider, velocity);
}

void Physics::Update(Body &body, float t, float dt,
                     const vector<SphereCollider> &dc, SphereCollider &col,
                     const SpacePartition &sp) {
  const vec3 prev_p = body.position;

  body.velocity += dt * grav_force;
  body.position += dt * body.velocity;

  // Test collisions at new body position
  col.position = body.position;

  // Instantly apply collisions to the velocity of the body
  computeCollisionRebound(sp, dc, col, body.velocity);

  // Adjust position based on (possibly) updated velocity
  body.position = prev_p + dt * body.velocity;

  // Match body's position with collider's
  col.position = body.position;
}
