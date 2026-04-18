#include "entities.h"

Entities::Entities() {
  RegisterEntities({{"assets/ramp1.gpmesh", BodyType::Static},
                    {"assets/snake1.gpmesh", BodyType::Static},
                    {"assets/plane1.gpmesh", BodyType::Static},
                    {"assets/plane2.gpmesh", BodyType::Static},
                    {"assets/plane3.gpmesh", BodyType::Static},
                    {"assets/plane4.gpmesh", BodyType::Static},
                    {"assets/plane5.gpmesh", BodyType::Static},
                    {"assets/plane6.gpmesh", BodyType::Static},
                    {"assets/plane7.gpmesh", BodyType::Static},
                    {"assets/big_ramp1.gpmesh", BodyType::Static},
                    {"assets/canon1.gpmesh", BodyType::Static},
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

  // Test collisions at new body position
  e.collider.position = e.body.position;

  // Instantly apply collisions to the velocity of the body
  Physics::getCollisionImpulse(sp, e.collider, e.body.velocity);

  // Adjust position based on (possibly) updated velocity
  e.body.position = prev_p + dt * e.body.velocity;

  // Match body's position with collider's
  e.collider.position = e.body.position;
}

void Entities::RegisterEntities(
    const vector<pair<string, BodyType>> &entityList) {
  for (const auto &[meshName, btype] : entityList) {
    auto maybe = Mesh::Load(meshName);

    if (maybe.has_value()) {
      auto [mesh, body] = maybe.value();

      if (btype == BodyType::Dynamic) {
        DynamicEntity de(mesh, body,
                         SphereCollider(body.position, body.scale.x));
        mDynamicEntities.emplace_back(de);
        mDynamicEntitiesStartingState.emplace_back(de);
      } else {
        auto triangles = Mesh::generateTriangleCollidersFromMesh(mesh, body);
        gSP.populate(triangles);

        mStaticEntities.emplace_back(mesh, body);
        mStaticColliders.emplace_back(triangles);
      }
    }
  }
}

vec3 &Entities::ProvideCameraFollow() {
  return mDynamicEntities[0].body.position;
}

void Entities::RegisterPlayerForward(float dt) {
  mDynamicEntities[0].body.velocity *= 1.0f + dt;
}

void Entities::RegisterPlayerLeft(float dt) {
  vec3 left = mDynamicEntities[0].body.velocity.cross(vec3::up).normalized();
  mDynamicEntities[0].body.velocity += left * 4.0f * dt;
}

void Entities::RegisterPlayerRight(float dt) {
  vec3 right = vec3::up.cross(mDynamicEntities[0].body.velocity).normalized();
  mDynamicEntities[0].body.velocity += right * 4.0f * dt;
}

void Entities::Restart() {
  for (int i = 0; i < mDynamicEntities.size(); i++) {
    mDynamicEntities[i] = mDynamicEntitiesStartingState[i];
  }
}
