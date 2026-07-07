#ifndef CAMERA_H
#define CAMERA_H
#pragma once

#include "maths.hpp"
#include "physics.hpp"

using Maths::mat4;
using Maths::vec3;
using Maths::vec4;

class FollowCamera {
  float mMouseDeltaX = 0.0f;
  float mMouseDeltaY = 0.0f;
  float mAngleHorizontal = 0.0f;
  float mAngleVertical = 25.0f * Maths::DegToRad;

public:
  vec3 mActualPosition;
  vec3 mActualTarget;
  vec3 mUp;
  vec3 mVelocity;
  float mSpringConstant;
  float mTargetDist;
  vec3 mIdealOffset;

  FollowCamera(const vec3 &target, const vec3 &up, const float spring,
               const float dist)
      : mActualTarget(target), mUp(up), mVelocity(vec3::zero),
        mSpringConstant(spring), mTargetDist(dist) {}

  // 描画が初めて起こる際に使う
  void Init() {
    const float ch = cosf(mAngleHorizontal);
    const float sh = sinf(mAngleHorizontal);
    const float cv = cosf(mAngleVertical);
    const float sv = sinf(mAngleVertical);

    mIdealOffset = mTargetDist * (vec3::up * sv -
                                  cv * (vec3::right * sh + vec3::forward * ch));

    SnapToTarget(mActualTarget);
  }

  void SetMouseMovement(float dx, float dy) {
    mMouseDeltaX = dx * Maths::DegToRad;
    mMouseDeltaY = dy * Maths::DegToRad;
  }

  void Update(float dt, const vec3 &follow, const vec3 &followForward) {
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

    const float ch = cosf(mAngleHorizontal);
    const float sh = sinf(mAngleHorizontal);
    const float cv = cosf(mAngleVertical);
    const float sv = sinf(mAngleVertical);

    mIdealOffset = mTargetDist * (vec3::up * sv -
                                  cv * (vec3::right * sh + vec3::forward * ch));

    // A higher value means the camera will take more
    // time to reach the ideal position
    // NOTE: Increasing the spring actually lowers dampening overall!
    // Dampening works against the spring constant (see accel below).
    float dampening = 2.0f * sqrt(mSpringConstant);

    const vec3 idealPosition = follow + mIdealOffset;
    const vec3 diff = mActualPosition - idealPosition;
    const vec3 accel = -mSpringConstant * diff - dampening * mVelocity;

    mVelocity += accel * dt;
    mActualPosition += mVelocity * dt;

    mActualTarget = follow;
  }

  void SnapToTarget(const vec3 &target) {
    mActualPosition = target + mIdealOffset;
    mActualTarget = target;
  }

  void ToCheckpoint(const vec3 &target) {
    mVelocity = vec3::zero;
    mActualTarget = target;

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

#endif // CAMERA_H
