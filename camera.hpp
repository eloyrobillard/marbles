#ifndef CAMERA_H
#define CAMERA_H

#include "maths.hpp"
#include "physics.hpp"

using Maths::mat4;
using Maths::vec3;
using Maths::vec4;

class ICamera {
public:
  virtual ~ICamera() = default;
  virtual void Update(float dt, const vec3 &target,
                      const vec3 &targetForward) = 0;
  virtual void HandleMouseMovement(float dx, float dy) = 0;
  virtual void HandleKeyboardLeft() = 0;
  virtual void HandleKeyboardRight() = 0;
  virtual void HandleKeyboardUp() = 0;
  virtual void HandleKeyboardDown() = 0;
  [[nodiscard]] virtual const vec3 &GetPosition() const = 0;
  [[nodiscard]] virtual const vec3 &GetTarget() const = 0;
};

class FollowCamera : public ICamera {
  float mMouseDeltaX = 0.0f;
  float mMouseDeltaY = 0.0f;
  const float mStartingAngleH = 0.0f;
  const float mStartingAngleV = 25.0f * Maths::DegToRad;
  float mAngleHorizontal = mStartingAngleH;
  float mAngleVertical = mStartingAngleV;

public:
  vec3 mActualPosition;
  vec3 mActualTarget;
  vec3 mVelocity;
  float mSpringConstant;
  float mTargetDist;
  vec3 mIdealOffset;
  vec3 mStartingOffset;

  [[nodiscard]] const vec3 &GetPosition() const override {
    return mActualPosition;
  }
  [[nodiscard]] const vec3 &GetTarget() const override { return mActualTarget; }

  ~FollowCamera() override = default;
  FollowCamera(const vec3 &target, const float spring, const float dist)
      : mActualTarget(target), mVelocity(vec3::zero), mSpringConstant(spring),
        mTargetDist(dist) {
    const float ch = cosf(mAngleHorizontal);
    const float sh = sinf(mAngleHorizontal);
    const float cv = cosf(mAngleVertical);
    const float sv = sinf(mAngleVertical);

    mStartingOffset =
        mTargetDist *
        (vec3::up * sv - cv * (vec3::right * sh + vec3::forward * ch));
    mIdealOffset = mStartingOffset;

    SnapToTarget(mActualTarget);
  }

  void HandleMouseMovement(float dx, float dy) override {
    mMouseDeltaX = dx * Maths::DegToRad;
    mMouseDeltaY = dy * Maths::DegToRad;
  }

  void HandleKeyboardLeft() override {}
  void HandleKeyboardRight() override {}
  void HandleKeyboardUp() override {}
  void HandleKeyboardDown() override {}

  [[nodiscard]] bool raycast(const vec3 &target) const {
    // レイキャストでカメラが壁に後ろを写すのを防ぐ
    const vec3 toTarget = target - mActualPosition;
    const float dist = toTarget.length();
    const vec3 unitToTarget = toTarget / dist;

    float t = 0.f;
    while (t <= dist) {
      const vec3 P = mActualPosition + t * unitToTarget;
      const vec3 Q = mActualPosition + (t + 0.25f) * unitToTarget;

      const auto staticColliders =
          gSpacePartition.getPartition(Q.x, Q.x, Q.y, Q.y, Q.z, Q.z);

      for (const auto &triangle : staticColliders) {
#ifdef _DEBUG
        gShowRaycastWireframe.push(triangle->vertexArray);
#endif

        if (Physics::segmentAndTriangleIntersect(P, Q, triangle->a, triangle->b,
                                                 triangle->c)) {
#ifdef _DEBUG
          gShowRaycastHit.push(triangle->vertexArray);
#endif
          return true;
        }
      }

      t += 0.25f;
    }

    return false;
  }

  void Update(float dt, const vec3 &target,
              const vec3 &targetForward) override {
    mAngleHorizontal = mAngleHorizontal + mMouseDeltaX;

    if (mAngleHorizontal > Maths::TAU) {
      mAngleHorizontal -= Maths::TAU;
    } else if (mAngleHorizontal < -Maths::TAU) {
      mAngleHorizontal += Maths::TAU;
    }

    mAngleVertical =
        std::clamp(mAngleVertical + mMouseDeltaY, -45.0f * Maths::DegToRad,
                   45.0f * Maths::DegToRad);

    // マウス入力による影響をリセット
    mMouseDeltaX = 0.0f;
    mMouseDeltaY = 0.0f;

    bool hit = raycast(target);

    // ユーザー入力がなかったときだけカメラを（ゆっくりと）動かす
    if (hit && mMouseDeltaX == 0.f && mMouseDeltaY == 0.f) {
      mAngleVertical += 0.005f;
    }

    const float ch = cosf(mAngleHorizontal);
    const float sh = sinf(mAngleHorizontal);
    const float cv = cosf(mAngleVertical);
    const float sv = sinf(mAngleVertical);

    mIdealOffset = mTargetDist * (vec3::up * sv -
                                  cv * (vec3::right * sh + vec3::forward * ch));

    // A higher value means the camera will take more
    // time to reach the ideal position
    // NOTE: Increasing the spring actually lowers dampening overall!
    // Dampening works against the spring constant.
    // See accel below for how the spring constant gets used.
    float dampening = 2.0f * sqrt(mSpringConstant);

    const vec3 idealPosition = target + mIdealOffset;
    const vec3 diff = mActualPosition - idealPosition;
    const vec3 accel = -mSpringConstant * diff - dampening * mVelocity;

    mVelocity += accel * dt;
    mActualPosition += mVelocity * dt;

    mActualTarget = Maths::lerp(target, target + targetForward * 2.0f, dt);
  }

  void SnapToTarget(const vec3 &target) {
    mActualPosition = target + mIdealOffset;
    mActualTarget = target;
  }

  void ToCheckpoint(const vec3 &target) {
    mVelocity = vec3::zero;
    mActualTarget = target;

    mIdealOffset = mStartingOffset;
    mAngleHorizontal = mStartingAngleH;
    mAngleVertical = mStartingAngleV;
    SnapToTarget(target);
  }

  [[nodiscard]] vec3 GetRight() const {
    return vec3::up.cross(mActualTarget - mActualPosition).normalized();
  }

  [[nodiscard]] vec3 GetForward() const {
    const vec3 toTarget = mActualTarget - mActualPosition;
    return vec3(toTarget.x, toTarget.y, 0.0f).normalized();
  }
};

class FreeCamera : public ICamera {
  float mMouseDeltaX = 0.0f;
  float mMouseDeltaY = 0.0f;
  const float mStartingAngleH = 0.0f;
  const float mStartingAngleV = 25.0f * Maths::DegToRad;
  float mAngleHorizontal = mStartingAngleH;
  float mAngleVertical = mStartingAngleV;
  vec3 up;
  vec3 position;
  vec3 target = vec3::forward;
  quat rotation;

public:
  FreeCamera(const vec3 &position, const vec3 &up)
      : up(up), position(position) {}

  ~FreeCamera() override = default;
  FreeCamera &operator=(FreeCamera &&o) noexcept {
    up = o.up;
    position = o.position;
    mAngleHorizontal = o.mAngleHorizontal;
    mAngleVertical = o.mAngleVertical;
    return *this;
  }

  void HandleMouseMovement(float dx, float dy) override {
    mMouseDeltaX = dx * Maths::DegToRad;
    mMouseDeltaY = dy * Maths::DegToRad;
  }

  void HandleKeyboardLeft() override {}
  void HandleKeyboardRight() override {}
  void HandleKeyboardUp() override {}
  void HandleKeyboardDown() override {}

  [[nodiscard]] const vec3 &GetPosition() const override { return position; }
  [[nodiscard]] const vec3 &GetTarget() const override { return target; }

  void Update(float dt, const vec3 &_target,
              const vec3 &_targetForward) override {
    mAngleHorizontal = mAngleHorizontal + mMouseDeltaX;

    if (mAngleHorizontal > Maths::TAU) {
      mAngleHorizontal -= Maths::TAU;
    } else if (mAngleHorizontal < -Maths::TAU) {
      mAngleHorizontal += Maths::TAU;
    }

    mAngleVertical =
        std::clamp(mAngleVertical - mMouseDeltaY, -45.0f * Maths::DegToRad,
                   45.0f * Maths::DegToRad);
    cout << mMouseDeltaY << endl;

    // マウス入力による影響をリセット
    mMouseDeltaX = 0.0f;
    mMouseDeltaY = 0.0f;

    const float ch = cosf(mAngleHorizontal);
    const float sh = sinf(mAngleHorizontal);
    const float cv = cosf(mAngleVertical);
    const float sv = sinf(mAngleVertical);

    target = vec3{ch * cv, sh * cv, sv};
    target.normalize();
  }
};

#endif // CAMERA_H
