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
                .pivotAxis = -vec3::up,
                .pivotPoint = vec3::zero,
                .angleBoundsDeg = {0.0f, 45.0f},
                .resistance = 1.f});
  RegisterDoor({.meshPath = "assets/doorL.gpmesh",
                .pivotAxis = vec3::up,
                .pivotPoint = vec3::zero,
                .angleBoundsDeg = {-45.0f, 0.0f},
                .resistance = 1.f});

  RegisterMarble({"assets/sphere.gpmesh"});

  mAveragePos = mMarbles[0].position;
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
      // NOTE: maxScale * minScaleFactor + (n - 1) * (maxScale - maxScale *
      // minScaleFactor / (n-1)) = maxScale
      float scaleStep =
          (mMaxMarbleScale - mMaxMarbleScale * mMinMarbleScaleFactor) /
          static_cast<float>(mMaxNumMarbles - 1);
      marble.collider.radius += scaleStep;
      marble.scale += vec3(scaleStep);

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

    auto staticsPartition =
        gSpacePartition.getPartition(min_x, max_x, min_y, max_y, min_z, max_z);

    Physics::processStaticCollisions(staticsPartition, marble);

    vector<pair<TriangleCollider<PivotBody>, mat4>> doors{};

    // NOTE: コライダーの位置・回転をフレームごとに更新
    // 法線ベクトルは前のフレームに更新した
    vector<pair<GLuint, mat4>> showDoorWireframe{};
    for (auto &door : mDoors) {
      for (auto &t : door.colliders) {
        mat4 model = t.body->getWorldTransform();
        auto &[ref, _] = doors.emplace_back(t, model);

        ref.a = vec3(vec4(ref.a, 1.0f) * model);
        ref.b = vec3(vec4(ref.b, 1.0f) * model);
        ref.c = vec3(vec4(ref.c, 1.0f) * model);

        float bias = 0.5f;
        float minX = std::min(ref.a.x, std::min(ref.b.x, ref.c.x)) - bias;
        float maxX = std::max(ref.a.x, std::max(ref.b.x, ref.c.x)) + bias;
        float minY = std::min(ref.a.y, std::min(ref.b.y, ref.c.y)) - bias;
        float maxY = std::max(ref.a.y, std::max(ref.b.y, ref.c.y)) + bias;
        float minZ = std::min(ref.a.z, std::min(ref.b.z, ref.c.z)) - bias;
        float maxZ = std::max(ref.a.z, std::max(ref.b.z, ref.c.z)) + bias;

        if (!(maxX < min_x || minX > max_x || maxY < min_y || minY > max_y ||
              maxZ < min_z || minZ > max_z)) {
          showDoorWireframe.emplace_back(ref.vertexArray, model);
        }
      }
    }

    Physics::processDoorCollisions(doors, marble);

    // マーブルの回転処理
    marble.rotationAxis = marble.rotationalVelocity.cross(vec3::up);

    // 回転軸をマーブルと逆に回転させると、Concat後正しい向きになる
    marble.rotationAxis = quat::RotateVector(quat::Conjugate(marble.rotation),
                                             marble.rotationAxis)
                              .normalized();

    quat deltaRot(marble.rotationAxis, marble.rotationalVelocity.length() /
                                           marble.collider.radius * deltaTime);

    marble.rotation = quat::Concatenate(marble.rotation, deltaRot);

    for (auto &door : mDoors) {
      // 抵抗力を適用
      door.body->rotationalVelocity =
          quat::Lerp(door.body->rotationalVelocity,
                     quat::Concatenate(door.body->rotationalVelocity,
                                       door.body->constantAcceleration),
                     deltaTime);

      // FIX: 後ろからドアにぶつかっても開いてしまう
      auto targetRot =
          quat::Concatenate(door.body->rotation, door.body->rotationalVelocity);

      auto finalRot = quat::Lerp(door.body->rotation, targetRot, deltaTime);

      // 回転の範囲を越えたら速度をリセット
      if (door.body->angleBounds.minSinHalfRad <= finalRot.z &&
          finalRot.z <= door.body->angleBounds.maxSinHalfRad) {
        door.body->rotation = finalRot;

        for (auto &t : door.colliders) {
          // TODO: 新しい法線の計算が妥当かどうか確認
          t.updateNormal();
        }
      } else {
        door.body->rotationalVelocity = quat::Identity;
      }
    }

#ifdef _DEBUG
    // Set vertices to be shown as wireframe
    vector<GLuint> showWireframe;
    std::ranges::transform(
        staticsPartition, std::back_inserter(showWireframe),
        [](const TriangleCollider<StaticBody> *s) { return s->vertexArray; });
    gShowWireframe = showWireframe;

    gShowDoorWireframe = showDoorWireframe;
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
    // NOTE: マーブル結合のときに使う
    mMaxMarbleScale = b.scale.x;
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
                doorData.angleBoundsDeg, doorData.resistance);
    shared_ptr<PivotBody> shB = std::make_shared<PivotBody>(b);
    auto triangles = mesh.generateTriangleCollidersFromMesh(shB);
    mDoors.emplace_back(mesh, shB, triangles);
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
                   datum.overrideSpeed,
                   vec3(vec4(datum.impulseOverride, 1.0f) * rot),
                   vec3(vec4(datum.speedOverride, 1.0f) * rot));

      shared_ptr<StaticBody> shB = std::make_shared<StaticBody>(b);
      auto triangles = mesh.generateTriangleCollidersFromMesh(shB);
      auto &ref = mStaticEntities.emplace_back(mesh, shB, triangles);

      gSpacePartition.populate(ref.colliders);
    }
  }
}

void Entities::RegisterInputForward(float dt, const vec3 &cameraForward) {
  if (mSplitMode == SplitMode::Joined)
    mMarbles[0].RegisterInputForward(dt, cameraForward);
}

void Entities::RegisterInputBackward(float dt, const vec3 &cameraForward) {
  if (mSplitMode == SplitMode::Joined)
    mMarbles[0].RegisterInputBackward(dt, cameraForward);
}

void Entities::RegisterInputLeft(float dt, const vec3 &cameraRight) {
  if (mSplitMode == SplitMode::Joined)
    mMarbles[0].RegisterInputLeft(dt, cameraRight);
}

void Entities::RegisterInputRight(float dt, const vec3 &cameraRight) {
  if (mSplitMode == SplitMode::Joined)
    mMarbles[0].RegisterInputRight(dt, cameraRight);
}

void Entities::ToCheckpoint(const vector<vec3> &positionsAtCheckpoint) {
  if (mSplitMode != SplitMode::Joined) {
    join();
  }

  mMarbles[0].ResetToPosition(positionsAtCheckpoint[0]);

  mAveragePos = mMarbles[0].position;
  mAverageVel = mMarbles[0].velocity;
}

void Entities::split() {
  mSplitMode = SplitMode::Split;
  mCurNumMarbles = mMaxNumMarbles;

  const vec3 &pos = mMarbles[0].position;
  const vec3 &vel = mMarbles[0].velocity;
  const float scale = mMarbles[0].scale.x;
  const float minScale = mMaxMarbleScale * mMinMarbleScaleFactor;

  for (int i = 0; i < mCurNumMarbles; i++) {
    // Place tiny marble at random position inside the sphere of the original
    // marble
    mMarbles[i].position =
        pos + vec3(0.0f, 0.0f, scale) +
        (vec3::rand(1.0f, 1.0f, 1.0f) - vec3(0.5f, 0.5f, 0.0f)) * scale;
    mMarbles[i].velocity = vel;
    mMarbles[i].collider.radius = minScale;
    mMarbles[i].scale = vec3(minScale);
  }
}

void Entities::join() {
  mSplitMode = SplitMode::Joined;

  mMarbles[0].position = mAveragePos;
  mMarbles[0].velocity = mAverageVel;
  mMarbles[0].collider.radius = mMaxMarbleScale;
  mMarbles[0].scale = vec3(mMaxMarbleScale);
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
