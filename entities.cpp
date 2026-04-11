#include "entities.h"

Entities::Entities() {
  RegisterEntities({{"assets/twist.gpmesh", BodyType::Static},
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
  Physics::computeCollisionRebound(sp, e.collider, e.body.velocity);

  // Adjust position based on (possibly) updated velocity
  e.body.position = prev_p + dt * e.body.velocity;

  // Match e.body's position with collider's
  e.collider.position = e.body.position;
}

void Entities::RegisterEntities(
    const vector<pair<string, BodyType>> &entityList) {
  for (const auto &[meshName, btype] : entityList) {
    auto maybe = Mesh::load(meshName);

    if (maybe.has_value()) {
      auto [mesh, body] = maybe.value();

      if (btype == BodyType::Dynamic) {
        mDynamicEntities.emplace_back(
            mesh, body, SphereCollider(body.position, body.scale.x));
      } else {
        auto triangles = Mesh::generateTriangleCollidersFromMesh(mesh, body);
        gSP.populate(triangles);

        // Store static objects at the front of the queue
        mStaticEntities.emplace_back(mesh, body);
        mStaticColliders.emplace_back(triangles);
      }
    }
  }
}

Body &Entities::ProvideCameraFollow() { return mDynamicEntities[0].body; }
