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

  mCurrentDynamicEntities = ranges::subrange{views::take(mDynamicEntities, 1)};
}

void Entities::Update(float time, float deltaTime) {
  for (auto &e : mCurrentDynamicEntities) {
    // Prepare new body position to test collisions at
    e.UpdateFirstPass(time, deltaTime);
  }

  // Instantly apply collisions to the velocity of the body
  GetCollisionImpulse();

  for (auto &e : mCurrentDynamicEntities) {
    // Adjust position based on (possibly) updated velocity
    e.UpdateSecondPass(time, deltaTime);
  }
}

void Entities::GetCollisionImpulse() {
  for (int i = 0; i < mCurrentDynamicEntities.size(); i++) {
    float min_x = mCurrentDynamicEntities[i].collider.position.x -
                  mCurrentDynamicEntities[i].collider.radius;
    float max_x = mCurrentDynamicEntities[i].collider.position.x +
                  mCurrentDynamicEntities[i].collider.radius;
    float min_y = mCurrentDynamicEntities[i].collider.position.y -
                  mCurrentDynamicEntities[i].collider.radius;
    float max_y = mCurrentDynamicEntities[i].collider.position.y +
                  mCurrentDynamicEntities[i].collider.radius;
    float min_z = mCurrentDynamicEntities[i].collider.position.z -
                  mCurrentDynamicEntities[i].collider.radius;
    float max_z = mCurrentDynamicEntities[i].collider.position.z +
                  mCurrentDynamicEntities[i].collider.radius;

    gCurrentPartition =
        gSpacePartition.get_partition(min_x, max_x, min_y, max_y, min_z, max_z);

    Physics::processStaticCollisions(
        gCurrentPartition, mCurrentDynamicEntities[i].collider,
        mCurrentDynamicEntities[i].GetVelocityAsRef());
  }
}

void DynamicEntity::UpdateFirstPass(float t, float dt) {
  mPrevPos = body.position;

  body.velocity += dt * grav_force;
  body.position += dt * body.velocity;

  collider.position = body.position;
}

void DynamicEntity::UpdateSecondPass(float t, float dt) {
  // Adjust position based on (possibly) updated velocity
  body.position = mPrevPos + dt * body.velocity;

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

void Entities::ToggleSplitMode() {
  mSplitMode = !mSplitMode;

  if (!mSplitMode) {
    const vec3 &pos = mDynamicEntities[1].GetPositionAsRef();
    const vec3 &vel = mDynamicEntities[1].GetVelocityAsRef();

    mDynamicEntities[0].SetPosition(pos);
    mDynamicEntities[0].SetVelocity(vel);

    mCurrentDynamicEntities =
        ranges::subrange{views::take(mDynamicEntities, 1)};
  } else {
    const vec3 &pos = mDynamicEntities[0].GetPositionAsRef();
    const vec3 &vel = mDynamicEntities[0].GetVelocityAsRef();

    for (int i = 1; i < mDynamicEntities.size(); i++) {
      mDynamicEntities[i].SetPosition(pos + vec3::rand(0.5f, 0.5f, 0.0f));
      mDynamicEntities[i].SetVelocity(vel);
    }

    mCurrentDynamicEntities =
        ranges::subrange{views::drop(mDynamicEntities, 1)};
  }
};
