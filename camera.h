#ifndef CAMERA_H
#define CAMERA_H
#pragma once

#include "physics.h"
#include "template.h"

using Tmpl8::mat4;
using Tmpl8::vec3;
using Tmpl8::vec4;

class FollowCamera {
  vec3 mStartingPosition;

public:
  vec3 mActualPosition;
  vec3 mTarget;
  vec3 mUp;
  vec3 mVelocity;
  float mTargetDist;
  float mSpringConstant;
  vec3 mIdealOffset;

  FollowCamera(const vec3 &startingFollowPosition, const vec3 &offset,
               const vec3 &target, const vec3 &up)
      : mIdealOffset(offset), mActualPosition(startingFollowPosition + offset),
        mStartingPosition(startingFollowPosition + offset), mTarget(target),
        mUp(up), mVelocity(vec3::zero), mTargetDist(1.0f),
        mSpringConstant(2.0f) {}

  void update(float dt, const vec3 &follow) {
    vec3 idealPosition = follow + mIdealOffset;

    float dampening = 2.0f * sqrt(mSpringConstant);

    vec3 diff = mActualPosition - idealPosition;
    vec3 accel = -mSpringConstant * diff - dampening * mVelocity;

    mVelocity += accel * dt;
    mActualPosition += mVelocity * dt;

    mTarget = follow + vec3::forward * mTargetDist;
  }

  void Restart(const vec3 &target) {
    mActualPosition = mStartingPosition;
    mVelocity = vec3::zero;
    mTarget = target;
  }
};

#endif // CAMERA_H
