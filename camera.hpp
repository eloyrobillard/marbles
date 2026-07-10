#ifndef CAMERA_H
#define CAMERA_H

#include "maths.hpp"
#include "physics.hpp"

using Maths::mat4;
using Maths::vec3;
using Maths::vec4;

class FollowCamera {
  float mMouseDeltaX = 0.0f;
  float mMouseDeltaY = 0.0f;
  const float mStartingAngleH = 0.0f;
  const float mStartingAngleV = 25.0f * Maths::DegToRad;
  float mAngleHorizontal = mStartingAngleH;
  float mAngleVertical = mStartingAngleV;

public:
  vec3 mActualPosition;
  vec3 mActualTarget;
  vec3 mUp;
  vec3 mVelocity;
  float mSpringConstant;
  float mTargetDist;
  vec3 mIdealOffset;
  vec3 mStartingOffset;

  FollowCamera(const vec3 &target, const vec3 &up, const float spring,
               const float dist)
      : mActualTarget(target), mUp(up), mVelocity(vec3::zero),
        mSpringConstant(spring), mTargetDist(dist) {
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

  void SetMouseMovement(float dx, float dy) {
    mMouseDeltaX = dx * Maths::DegToRad;
    mMouseDeltaY = dy * Maths::DegToRad;
  }

  // SOURCE: Real-Time Collision Detection by Christer Ericson (5.3.6)
  static bool segmentAndTriangleIntersect(const vec3 &P, const vec3 &Q,
                                          const vec3 &A, const vec3 &B,
                                          const vec3 &C) {
    const vec3 QP = P - Q;
    const vec3 AC = C - A;
    const vec3 AB = B - A;

    const vec3 n = AB.cross(AC);

    const f32 d = QP.dot(n);
    if (d <= 0.f)
      return false;

    const vec3 AP = P - A;
    const f32 t = AP.dot(n);
    if (t < 0.f || t > d)
      return false;

    const vec3 e = QP.cross(AP);
    const f32 v = AC.dot(e);
    if (v < 0.f || v > d)
      return false;
    const f32 w = -AB.dot(e);
    if (w < 0.f || v + w > d)
      return false;

    return true;
  }

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

        if (segmentAndTriangleIntersect(P, Q, triangle->a, triangle->b,
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

  void Update(float dt, const vec3 &target, const vec3 &targetForward) {
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

#endif // CAMERA_H
