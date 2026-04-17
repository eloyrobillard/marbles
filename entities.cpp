#include "entities.h"

Entities::Entities() {
  RegisterEntities({{"assets/basic_ramp.gpmesh", BodyType::Static},
                    {"assets/sphere.gpmesh", BodyType::Dynamic}});
}

void Entities::Update(float time, float deltaTime) {
  for (auto &dynamicEntity : mDynamicEntities) {
    UpdateBody(time, deltaTime, dynamicEntity, gSP);
  }
}

void Entities::UpdateBody(float t, float dt, DynamicEntity &e,
                          const SpacePartition &sp) {
  const vec3 prev_p = e.body.position;

  e.body.velocity += dt * grav_force;
  e.body.position += dt * e.body.velocity;

  // Test collisions at new e.body position
  e.collider.position = e.body.position;

  // Instantly apply collisions to the velocity of the e.body
  Physics::getCollisionImpulse(sp, e.collider, e.body.velocity);

  // Adjust position based on (possibly) updated velocity
  e.body.position = prev_p + dt * e.body.velocity;

  // Match e.body's position with collider's
  e.collider.position = e.body.position;
}

void Entities::RegisterEntities(
    const vector<pair<string, BodyType>> &entityList) {
  for (const auto &[meshName, btype] : entityList) {
    auto maybe = Mesh::Load(meshName);

    if (maybe.has_value()) {
      auto [mesh, body] = maybe.value();

      if (btype == BodyType::Dynamic) {
        mDynamicEntities.emplace_back(
            mesh, body, SphereCollider(body.position, body.scale.x));
      } else {
        auto triangles = Mesh::generateTriangleCollidersFromMesh(mesh, body);
        gSP.populate(triangles);

        mStaticEntities.emplace_back(mesh, body);
        mStaticColliders.emplace_back(triangles);
      }
    }
  }
}

Body &Entities::ProvideCameraFollow() { return mDynamicEntities[0].body; }

void Entities::RegisterPlayerForward() {
  mDynamicEntities[0].body.velocity *= 1.001f;
}

void Entities::RegisterPlayerLeft() {
  vec3 left = mDynamicEntities[0].body.velocity.cross(vec3::up).normalized();
  mDynamicEntities[0].body.velocity += left * 0.3f;
}

void Entities::RegisterPlayerRight() {
  vec3 right = vec3::up.cross(mDynamicEntities[0].body.velocity).normalized();
  mDynamicEntities[0].body.velocity += right * 0.3f;
}
