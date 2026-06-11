#include "entities.hpp"
#include "physics.hpp"

StaticEntityData GenerateEntityData(string meshPath,
                                    float collisionAcceleration = 1.0f,
                                    bool overrideImpulse = false,
                                    vec3 impulseOverride = vec3::zero) {
  return {std::move(meshPath), collisionAcceleration, overrideImpulse,
          impulseOverride};
}

Entities::Entities() {
  mMarbles.reserve(mMaxNumMarbles);
  mPreviousPositions.reserve(mMaxNumMarbles);

  RegisterStaticEntities({
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
      {.meshPath = "assets/canon1.gpmesh",
       .collisionAcceleration = 1.03f,
       .overrideImpulse = true,
       .impulseOverride = vec3::up},
  });

  RegisterMarble({"assets/sphere.gpmesh"});
}

void Entities::Update(float time, float deltaTime) {
  // MUST BE DONE HERE
  computeAveragePosition();
  computeAverageVelocity();
  computePositionalVariance();
  computeAveragePositionWithoutOutliers();

  for (int i = 0; i < (mSplitMode ? mCurNumMarbles : 1); i++) {
    DynamicBody &m = mMarbles[i];

    // Prepare new body position to test collisions at
    mPreviousPositions[i] = m.position;

    m.velocity += deltaTime * grav_force;
    m.position += deltaTime * m.velocity;

    m.collider.position = m.position;

    // Instantly apply collisions to the velocity of the body
    Physics::processDynamicCollisions(mMarbles, i);

    // Adjust position based on (possibly) updated velocity
    m.position = mPreviousPositions[i] + deltaTime * m.velocity;

    // Match m's position with collider's
    m.collider.position = m.position;

    float min_x = m.collider.position.x - m.collider.radius;
    float max_x = m.collider.position.x + m.collider.radius;
    float min_y = m.collider.position.y - m.collider.radius;
    float max_y = m.collider.position.y + m.collider.radius;
    float min_z = m.collider.position.z - m.collider.radius;
    float max_z = m.collider.position.z + m.collider.radius;

    gCurrentPartition =
        gSpacePartition.get_partition(min_x, max_x, min_y, max_y, min_z, max_z);

    Physics::processStaticCollisions(gCurrentPartition, m.collider, m.velocity);

    // Adjust position (again)
    m.position = mPreviousPositions[i] + deltaTime * m.velocity;

    // Match m's position with collider's
    m.collider.position = m.position;
  }
}

void Entities::RegisterMarble(const DynamicEntityData &entityData) {
  auto maybe = Mesh::Load(entityData.meshPath);

  if (maybe.has_value()) {
    auto [mesh, body] = maybe.value();
    body.scale *= entityData.scale;

    SphereCollider c(body.position, body.scale.x);
    DynamicBody b(c);
    b.scale = body.scale;
    b.position = body.position;

    mMarbleMesh = {mesh};

    for (int i = 0; i < mMaxNumMarbles; i++) {
      mMarbles.emplace_back(b);
      mPreviousPositions.emplace_back(b.position);
    }

    mDynamicEntitiesStartingState.emplace_back(b);
  }
}

void Entities::RegisterStaticEntities(
    const vector<StaticEntityData> &entityList) {
  for (const auto &[mesh_name, accel, override_impulse, impulse_override,
                    scale] : entityList) {
    auto maybe = Mesh::Load(mesh_name);

    if (maybe.has_value()) {
      auto [mesh, body] = maybe.value();
      body.scale *= scale;

      auto triangles = mesh.generateTriangleCollidersFromMesh(
          body, accel, override_impulse, impulse_override);
      gSpacePartition.populate(triangles);

      StaticBody b(body);
      mStaticEntities.emplace_back(mesh, b);
      mStaticColliders.emplace_back(triangles);
    }
  }
}

void Entities::RegisterInputLeft(float dt) {
  if (!mSplitMode)
    mMarbles[0].RegisterInputLeft(dt);
}

void Entities::RegisterInputRight(float dt) {
  if (!mSplitMode)
    mMarbles[0].RegisterInputRight(dt);
}

void Entities::ToCheckpoint(const vector<vec3> &positionsAtCheckpoint) {
  if (mSplitMode) {
    join();
    mSplitMode = false;
  }

  mMarbles[0].ResetToPosition(positionsAtCheckpoint[0]);
}

void Entities::split() {
  const vec3 &pos = mMarbles[0].position;
  const vec3 &vel = mMarbles[0].velocity;
  const float radius =
      mMarbles[0].collider.radius / static_cast<float>(mCurNumMarbles);

  for (int i = 0; i < mCurNumMarbles; i++) {
    mMarbles[i].position =
        pos + vec3::rand(1.0f, 1.0f, 0.5f) - vec3(0.5f, 0.5f, 0.0f);
    mMarbles[i].velocity = vel;
    mMarbles[i].collider.radius = radius;
    mMarbles[i].scale = vec3(radius);
  }
}

void Entities::join() {
  mMarbles[0].position = mAveragePos;
  mMarbles[0].velocity = mAverageVel;

  float newRadius =
      mMarbles[0].collider.radius * static_cast<float>(mCurNumMarbles);
  mMarbles[0].collider.radius = newRadius;
  mMarbles[0].scale = vec3(newRadius);
}

void Entities::ToggleSplitMode() {
  mSplitMode = !mSplitMode;

  if (!mSplitMode)
    join();
  else
    split();
};
