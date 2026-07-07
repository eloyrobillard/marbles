#ifndef CAMERA_H
#define CAMERA_H
#pragma once

#include "maths.hpp"
#include "physics.hpp"

using Maths::mat4;
using Maths::vec3;
using Maths::vec4;

class FollowCamera {
  vec3 mStartingPosition;
  float mMouseDeltaX = 0.0f;
  float mMouseDeltaY = 0.0f;
  float mAngleHorizontal = 0.0f;
  float mAngleVertical = 0.0f;

public:
  vec3 mActualPosition;
  vec3 mActualTarget;
  vec3 mUp;
  vec3 mVelocity;
  float mTargetDist;
  float mSpringConstant;
  vec3 mIdealOffset;

  FollowCamera(const vec3 &startingFollowPosition, const vec3 &offset,
               const vec3 &target, const vec3 &up, const float spring)
      : mIdealOffset(offset), mActualPosition(startingFollowPosition + offset),
        mStartingPosition(startingFollowPosition + offset),
        mActualTarget(target), mUp(up), mVelocity(vec3::zero),
        mTargetDist(6.0f), mSpringConstant(spring) {}

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

    mActualPosition =
        follow + mTargetDist * (vec3::up * sv -
                                cv * (vec3::right * sh + vec3::forward * ch));

    mActualTarget = follow;
  }

  void SnapToTarget(const vec3 &target) {
    mActualPosition = target + mIdealOffset;
  }

  void ToCheckpoint(const vec3 &target) {
    mVelocity = vec3::zero;
    mActualTarget = target;

    SnapToTarget(target);
  }
};

#endif // CAMERA_H
