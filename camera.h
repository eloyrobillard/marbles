#ifndef CAMERA_H
#define CAMERA_H
#pragma once

#include "physics.h"
#include "template.h"

using Tmpl8::vec3;

class FollowCamera {
public:
  vec3 mActualPosition;
  vec3 mTarget;
  vec3 mUp;
  vec3 mVelocity;
  float mTargetDist;
  float mSpringConstant;

  FollowCamera(const vec3 &actualPosition, const vec3 &target, const vec3 &up)
      : mActualPosition(actualPosition), mTarget(target), mUp(up),
        mVelocity(vec3::zero), mTargetDist(1.0f), mSpringConstant(1.0f) {}

  void update(Body &follow, float dt) {
    vec3 idealPosition = follow.position + vec3(-5, 0, 3);

    float dampening = 2.0f * sqrt(mSpringConstant);

    vec3 diff = mActualPosition - idealPosition;
    vec3 accel = -mSpringConstant * diff - dampening * mVelocity;

    mVelocity += accel * dt;
    mActualPosition += mVelocity * dt;

    mTarget = follow.position + vec3::forward * mTargetDist;
  }
};

#endif // CAMERA_H
