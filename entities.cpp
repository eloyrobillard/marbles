#include "entities.hpp"

EntityData GenerateEntityData(string meshPath,
                              BodyType bodyType = BodyType::Static,
                              float collisionAcceleration = 1.0f,
                              bool overrideImpulse = false,
                              vec3 impulseOverride = vec3::zero) {
  return {std::move(meshPath), bodyType, collisionAcceleration, overrideImpulse,
          impulseOverride};
}

Entities::Entities() {
  RegisterEntities({
      {"assets/ramp1.gpmesh"},
      {"assets/ramp2.gpmesh"},
      {"assets/ramp3.gpmesh"},
      {"assets/snake1.gpmesh"},
      {"assets/plane1.gpmesh"},
      {"assets/plane2.gpmesh"},
      {"assets/plane3.gpmesh"},
      {"assets/plane4.gpmesh"},
      {"assets/plane5.gpmesh"},
      {"assets/plane6.gpmesh"},
      {"assets/plane7.gpmesh"},
      {"assets/plane8.gpmesh"},
      {.meshPath = "assets/canon1.gpmesh", .collisionAcceleration = 1.03f},
      {"assets/sphere.gpmesh", BodyType::Dynamic},
      {.meshPath = "assets/sphere.gpmesh",
       .bodyType = BodyType::Dynamic,
       .scale = vec3(0.5f)},
      {.meshPath = "assets/sphere.gpmesh",
       .bodyType = BodyType::Dynamic,
       .scale = vec3(0.5f)},
  });
}

void Entities::Update(float time, float deltaTime) {
  if (!mSplitMode) {
    mDynamicEntities[0].Update(time, deltaTime, gSpacePartition);
  } else {
    for (int i = 1; i < mDynamicEntities.size(); i++) {
      mDynamicEntities[i].Update(time, deltaTime, gSpacePartition);
    }
  }
}

void DynamicEntity::Update(float t, float dt, const SpacePartition &sp) {
  const vec3 prev_p = body.position;

  body.velocity += dt * grav_force;
  body.position += dt * body.velocity;

  // Test collisions at new body position
  collider.position = body.position;

  // Instantly apply collisions to the velocity of the body
  Physics::getCollisionImpulse(sp, collider, body.velocity);

  // Adjust position based on (possibly) updated velocity
  body.position = prev_p + dt * body.velocity;

  // Match body's position with collider's
  collider.position = body.position;
}

tuple<mat4, optional<Texture *>, GLuint, size_t> Entity::GetDrawData() const {
  return {body.getWorldTransform(), mesh.lookTextureUp(0),
          mesh.GetVertexArray(), mesh.GetNumIndices()};
}

void Entities::RegisterEntities(const vector<EntityData> &entityList) {
  for (const auto &[mesh_name, btype, accel, override_impulse, impulse_override,
                    scale] : entityList) {
    auto maybe = Mesh::Load(mesh_name);

    if (maybe.has_value()) {
      auto [mesh, body] = maybe.value();
      body.scale *= scale;

      if (btype == BodyType::Dynamic) {
        DynamicEntity de(mesh, body,
                         SphereCollider(body.position, body.scale.x));
        mDynamicEntities.emplace_back(de);
        mDynamicEntitiesStartingState.emplace_back(de);
      } else {
        auto triangles = mesh.generateTriangleCollidersFromMesh(
            body, accel, override_impulse, impulse_override);
        gSpacePartition.populate(triangles);

        mStaticEntities.emplace_back(mesh, body);
        mStaticColliders.emplace_back(triangles);
      }
    }
  }
}

void Entities::RegisterInputLeft(float dt) {
  if (!mSplitMode)
    mDynamicEntities[0].RegisterInputLeft(dt);
}

void Entities::RegisterInputRight(float dt) {
  if (!mSplitMode)
    mDynamicEntities[0].RegisterInputRight(dt);
}

void Entities::ToCheckpoint(const vector<vec3> &positionsAtCheckpoint) {
  for (int i = 0; i < positionsAtCheckpoint.size(); i++) {
    mDynamicEntities[i].ResetToPosition(positionsAtCheckpoint[i]);
  }
}
