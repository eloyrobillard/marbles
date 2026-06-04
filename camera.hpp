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
        mTargetDist(3.0f), mSpringConstant(spring) {}

  void Update(float dt, const vec3 &follow, const vec3 &followVelocity) {
    vec3 followForward = followVelocity.normalized();
    vec3 followRight = {-followForward.y, followForward.x, 0};

    // TODO: fit offset to follow's forward direction
    vec3 idealPosition = follow + followForward * mIdealOffset.x +
                         followRight * mIdealOffset.y +
                         vec3::up * mIdealOffset.z;

    // A higher value means the camera will take more time to reach the ideal
    // position
    float dampening = 2.0f * sqrt(mSpringConstant);

    vec3 diff = mActualPosition - idealPosition;
    vec3 accel = -mSpringConstant * diff - dampening * mVelocity;

    mVelocity += accel * dt;
    mActualPosition += mVelocity * dt;

    vec3 idealTarget = follow + followForward * mTargetDist;

    mActualTarget = Maths::lerp(mActualTarget, idealTarget, 3 * dt);
  }

  void SnapToTarget(const vec3 &target) {
    mActualPosition = mActualTarget + mIdealOffset;
  }

  void ToCheckpoint(const vec3 &target) {
    mVelocity = vec3::zero;
    mActualTarget = target;

    SnapToTarget(target);
  }
};

#endif // CAMERA_H
