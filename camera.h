#ifndef CAMERA_H
#define CAMERA_H
#pragma once

#include "physics.h"
#include "template.h"

using Tmpl8::mat4;
using Tmpl8::vec3;
using Tmpl8::vec4;

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
        mVelocity(vec3::zero), mTargetDist(1.0f), mSpringConstant(2.0f) {}

  void update(float dt, Body &follow) {
    vec3 idealOffset = vec3(-3.0f, 0.0f, 3.0f);
    vec3 idealPosition = follow.position + idealOffset;

    float dampening = 2.0f * sqrt(mSpringConstant);

    vec3 diff = mActualPosition - idealPosition;
    vec3 accel = -mSpringConstant * diff - dampening * mVelocity;

    mVelocity += accel * dt;
    mActualPosition += mVelocity * dt;

    mTarget = follow.position + vec3::forward * mTargetDist;
  }
};

#endif // CAMERA_H
