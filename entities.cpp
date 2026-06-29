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
      {"assets/hingeR.gpmesh"},
      {"assets/hingeL.gpmesh"},
      {"assets/plane5.gpmesh"},
      {"assets/plane7.gpmesh"},
      {"assets/twist3.gpmesh"},
      {.meshPath = "assets/canon1.gpmesh",
       .overrideSpeed = true,
       .speedOverride = vec3::up * 50.0f},
  });

  RegisterDoor({.meshPath = "assets/doorR.gpmesh",
                .pivotAxis = vec3::up,
                .pivotPoint = vec3::zero,
                .maxAngle = 45,
                .resistance = 10.0f});
  RegisterDoor({.meshPath = "assets/doorL.gpmesh",
                .pivotAxis = vec3::up,
                .pivotPoint = vec3::zero,
                .maxAngle = -45,
                .resistance = 10.0f});

  RegisterMarble({"assets/sphere.gpmesh"});
}

void Entities::Update(float time, float deltaTime) {
  // MUST BE DONE HERE
  computeAveragePosition();
  computeAverageVelocity();
  computePositionalVariance();
  computeAveragePositionWithoutOutliers();

  for (int i = 0; i < getNumMarbles(); i++) {
    DynamicBody &marble = mMarbles[i];

    // Prepare new body position to test collisions at
    mPreviousPositions[i] = marble.position;

    if (mSplitMode == SplitMode::Joining) {
      // Once all marbles are done fusing into one, return to single marble mode
      if (mCurNumMarbles == 1) {
        join();
      } else {
        vec3 to = mAveragePos - marble.position;
        marble.velocity += to / 64.0f;
      }
    }

    marble.velocity += deltaTime * gGravity;
    marble.position += deltaTime * marble.velocity;

    marble.collider.position = marble.position;

    // Instantly apply collisions to the velocity of the body
    // If in joining mode, instantly join with any collided marble
    int collisionIdx = Physics::processDynamicCollisions(
        mMarbles, i, mCurNumMarbles, mSplitMode == SplitMode::Joining);

    if (collisionIdx > -1) {
      vec3 averagePos =
          (marble.position + mMarbles[collisionIdx].position) / 2.0f;
      marble.collider.radius += mMarbles[collisionIdx].collider.radius;
      marble.scale += mMarbles[collisionIdx].scale;

      mCurNumMarbles--;
      mMarbles[collisionIdx] = mMarbles[mCurNumMarbles];
    }

    // Adjust position based on (possibly) updated velocity
    marble.position = mPreviousPositions[i] + deltaTime * marble.velocity;

    // Match m's position with collider's
    marble.collider.position = marble.position;

    float min_x = marble.collider.position.x - marble.collider.radius;
    float max_x = marble.collider.position.x + marble.collider.radius;
    float min_y = marble.collider.position.y - marble.collider.radius;
    float max_y = marble.collider.position.y + marble.collider.radius;
    float min_z = marble.collider.position.z - marble.collider.radius;
    float max_z = marble.collider.position.z + marble.collider.radius;

    auto statics =
        gSpacePartition.get_partition(min_x, max_x, min_y, max_y, min_z, max_z);

    Physics::processStaticCollisions(statics, marble);

    auto doors =
        gDoorSP.get_partition(min_x, max_x, min_y, max_y, min_z, max_z);

    Physics::processDoorCollisions(doors, marble);

    for (auto &door : doors) {
      // TODO: figure out quaternion/slerp
      auto targetRot =
          quat::Concatenate(door.body->rotation, door.body->rotationalVelocity);
      door.body->rotation =
          quat::Lerp(door.body->rotation, targetRot, deltaTime);
    }

#ifdef _DEBUG
    // Set vertices to be shown as wireframe
    // FIX: vertex array だけ渡すと、回転などは反映されない
    vector<GLuint> showWireframe;
    std::ranges::transform(
        statics, std::back_inserter(showWireframe),
        [](const TriangleCollider<StaticBody> &s) { return s.vertexArray; });
    std::ranges::transform(
        doors, std::back_inserter(showWireframe),
        [](const TriangleCollider<PivotBody> &d) { return d.vertexArray; });
    gShowWireframe = showWireframe;
#endif

    // Adjust position (again)
    marble.position = mPreviousPositions[i] + deltaTime * marble.velocity;

    // Match m's position with collider's
    marble.collider.position = marble.position;
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

void Entities::RegisterDoor(const DoorData &doorData) {
  auto maybe = Mesh::Load(doorData.meshPath);

  if (maybe.has_value()) {
    auto [mesh, body] = maybe.value();
    body.scale *= doorData.scale;

    PivotBody b(body, doorData.pivotAxis, doorData.pivotPoint,
                doorData.maxAngle, doorData.resistance);
    shared_ptr<PivotBody> shB = std::make_shared<PivotBody>(b);
    auto triangles = mesh.generateTriangleCollidersFromMesh<PivotBody>(shB);
    gDoorSP.populate(triangles);

    mDoors.emplace_back(mesh, shB);
  }
}

void Entities::RegisterStaticEntities(const vector<StaticEntityData> &data) {
  for (const auto &datum : data) {
    auto maybe = Mesh::Load(datum.meshPath);

    if (maybe.has_value()) {
      auto [mesh, body] = maybe.value();
      body.scale *= datum.scale;

      const mat4 rot = mat4::CreateFromQuaternion(body.rotation);

      StaticBody b(body, datum.collisionAcceleration, datum.overrideImpulse,
                   datum.overrideSpeed, vec4(datum.impulseOverride, 1.0f) * rot,
                   vec4(datum.speedOverride, 1.0f) * rot);

      shared_ptr<StaticBody> shB = std::make_shared<StaticBody>(b);
      auto triangles = mesh.generateTriangleCollidersFromMesh(shB);
      gSpacePartition.populate(triangles);

      mStaticEntities.emplace_back(mesh, shB);
    }
  }
}

void Entities::RegisterInputLeft(float dt) {
  if (mSplitMode == SplitMode::Joined)
    mMarbles[0].RegisterInputLeft(dt);
}

void Entities::RegisterInputRight(float dt) {
  if (mSplitMode == SplitMode::Joined)
    mMarbles[0].RegisterInputRight(dt);
}

void Entities::ToCheckpoint(const vector<vec3> &positionsAtCheckpoint) {
  if (mSplitMode != SplitMode::Joined) {
    join();
  }

  mMarbles[0].ResetToPosition(positionsAtCheckpoint[0]);
}

void Entities::split() {
  mSplitMode = SplitMode::Split;
  mCurNumMarbles = mMaxNumMarbles;

  const vec3 &pos = mMarbles[0].position;
  const vec3 &vel = mMarbles[0].velocity;
  const float scale = mMarbles[0].scale.x;
  const float radius =
      mMarbles[0].collider.radius / static_cast<float>(mCurNumMarbles);

  for (int i = 0; i < mCurNumMarbles; i++) {
    // Place tiny marble at random position inside the sphere of the original
    // marble
    mMarbles[i].position =
        pos + vec3(0.0f, 0.0f, scale) +
        (vec3::rand(1.0f, 1.0f, 1.0f) - vec3(0.5f, 0.5f, 0.0f)) * scale;
    mMarbles[i].velocity = vel;
    mMarbles[i].collider.radius = radius;
    mMarbles[i].scale = vec3(radius);
  }
}

void Entities::join() {
  mSplitMode = SplitMode::Joined;

  mMarbles[0].position = mAveragePos;
  mMarbles[0].velocity = mAverageVel;

  for (int i = 1; i < mCurNumMarbles; i++) {
    mMarbles[0].collider.radius += mMarbles[i].collider.radius;
    mMarbles[0].scale += mMarbles[i].scale;
  }
}

void Entities::ToggleSplitMode() {
  if (mSplitMode == SplitMode::Split)
    startJoin();
  else if (mSplitMode == SplitMode::Joining)
    stopJoin();
  else
    split();
}

void Entities::computeAveragePosition() {
  vec3 res = vec3(0.0f);

  int numMarbles = getNumMarbles();

  for (int i = 0; i < numMarbles; i++) {
    res += mMarbles[i].position;
  }

  mAveragePos = res / static_cast<float>(numMarbles);
}

void Entities::computeAveragePositionWithoutOutliers() {
  vec3 res = vec3(0.0f);

  int numMarbles = getNumMarbles();

  for (int i = 0; i < numMarbles; i++) {
    float distSqrd = mAveragePos.distanceSqrd(mMarbles[i].position);

    if (distSqrd <= 3 * mPositionalVariance) {
      res += mMarbles[i].position;
    }
  }

  mAveragePos = res / static_cast<float>(numMarbles);
}

void Entities::computePositionalVariance() {
  mPositionalVariance = 0;

  int numMarbles = getNumMarbles();

  for (int i = 0; i < numMarbles; i++) {
    mPositionalVariance += mAveragePos.distanceSqrd(mMarbles[i].position);
  }

  mPositionalVariance /= static_cast<float>(numMarbles);
}

void Entities::computeAverageVelocity() {
  vec3 res = vec3(0.0f);

  int numMarbles = getNumMarbles();

  for (int i = 0; i < numMarbles; i++) {
    res += mMarbles[i].velocity;
  }

  mAverageVel = res / static_cast<float>(numMarbles);
}
