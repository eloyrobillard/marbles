#ifndef _PHYSICS_H
#define _PHYSICS_H
#pragma once

#include "colliders.hpp"
#include "entities.hpp"
#include "maths.hpp"
#include "pch.h"

using Maths::mat4;
using Maths::quat;
using Maths::vec3;

const vec3 gGravity = vec3(0.0f, 0.0f, -9.81f);

template <class T>
  requires std::derived_from<T, Body>
class SpacePartition {
  float mMinX;
  float mMaxX;
  float mMinY;
  float mMaxY;
  float mMinZ;
  float mMaxZ;
  float mStep;
  size_t mNumX;
  size_t mNumY;
  size_t mNumZ;
  vector<vector<TriangleCollider<T> *>> mPartition;

  void populate(const TriangleCollider<T> &tc, float min_x, float max_x,
                float min_y, float max_y, float min_z, float max_z);

public:
  SpacePartition(float min_x, float max_x, float min_y, float max_y,
                 float min_z, float max_z, float step)
      : mMinX(min_x), mMaxX(max_x), mMinY(min_y), mMaxY(max_y), mMinZ(min_z),
        mMaxZ(max_z), mStep(step) {
    mNumX = ceil((max_x - min_x) / step);
    mNumY = ceil((max_y - min_y) / step);
    mNumZ = ceil((max_z - min_z) / step);

    mPartition = vector<vector<TriangleCollider<T> *>>(mNumX * mNumY * mNumZ);
  }

  SpacePartition(float min_x, float max_x, float min_y, float max_y,
                 float min_z, float max_z, float step,
                 const vector<TriangleCollider<T> *> &v)
      : mMinX(min_x), mMaxX(max_x), mMinY(min_y), mMaxY(max_y), mMinZ(min_z),
        mMaxZ(max_z), mStep(step) {
    mNumX = ceil((max_x - min_x) / step);
    mNumY = ceil((max_y - min_y) / step);
    mNumZ = ceil((max_z - min_z) / step);

    mPartition = vector<vector<TriangleCollider<T> *>>(mNumX * mNumY * mNumZ);

    populate(v);
  }

  ~SpacePartition() = default;

  void populate(vector<TriangleCollider<T>> &v) {
    for (auto &tc : v) {
      float min_x = fmin(fmin(tc.a.x, tc.b.x), tc.c.x);
      float max_x = fmax(fmax(tc.a.x, tc.b.x), tc.c.x);
      float min_y = fmin(fmin(tc.a.y, tc.b.y), tc.c.y);
      float max_y = fmax(fmax(tc.a.y, tc.b.y), tc.c.y);
      float min_z = fmin(fmin(tc.a.z, tc.b.z), tc.c.z);
      float max_z = fmax(fmax(tc.a.z, tc.b.z), tc.c.z);

      auto start_x =
          std::max((size_t)0, static_cast<size_t>((min_x - mMinX) / mStep));
      auto end_x =
          std::min(mNumX - 1, static_cast<size_t>((max_x - mMinX) / mStep));
      auto start_y =
          std::max((size_t)0, static_cast<size_t>((min_y - mMinY) / mStep));
      auto end_y =
          std::min(mNumY - 1, static_cast<size_t>((max_y - mMinY) / mStep));
      auto start_z =
          std::max((size_t)0, static_cast<size_t>((min_z - mMinZ) / mStep));
      auto end_z =
          std::min(mNumZ - 1, static_cast<size_t>((max_z - mMinZ) / mStep));

      for (size_t x = start_x; x <= end_x; x++) {
        for (size_t y = start_y; y <= end_y; y++) {
          for (size_t z = start_z; z <= end_z; z++) {
            mPartition[x * mNumY * mNumZ + y * mNumZ + z].push_back(&tc);
          }
        }
      }
    }
  }

  [[nodiscard]] vector<TriangleCollider<T> *>
  getPartition(float min_x, float max_x, float min_y, float max_y, float min_z,
               float max_z) const {
    vector<TriangleCollider<T> *> result{};
    auto start_x =
        std::max((size_t)0, static_cast<size_t>((min_x - mMinX) / mStep));
    size_t end_x =
        std::min(mNumX - 1, static_cast<size_t>((max_x - mMinX) / mStep));
    auto start_y =
        std::max((size_t)0, static_cast<size_t>((min_y - mMinY) / mStep));
    size_t end_y =
        std::min(mNumY - 1, static_cast<size_t>((max_y - mMinY) / mStep));
    auto start_z =
        std::max((size_t)0, static_cast<size_t>((min_z - mMinZ) / mStep));
    size_t end_z =
        std::min(mNumZ - 1, static_cast<size_t>((max_z - mMinZ) / mStep));

    for (size_t x = start_x; x <= end_x; x++) {
      for (size_t y = start_y; y <= end_y; y++) {
        for (size_t z = start_z; z <= end_z; z++) {
          result.append_range(mPartition[x * mNumY * mNumZ + y * mNumZ + z]);
        }
      }
    }

    return result;
  };
};

// Used for spatial partitioning of static colliders
inline SpacePartition gSpacePartition = SpacePartition<StaticBody>(
    4.0f, 150.0f, -40.0f, 80.0f, -100.0f, 10.0f, 0.5f);

class DynamicEntity;

namespace Physics {
bool processDoorCollisions(
    const vector<pair<TriangleCollider<PivotBody>, mat4>> &triangles,
    DynamicBody &marble);
bool processStaticCollisions(
    const vector<TriangleCollider<StaticBody> *> &triangles,
    DynamicBody &marble);
int processDynamicCollisions(vector<DynamicBody> &des, int idx, int numMarbles,
                             bool joining);

inline float physicsTicksPerSecond = 240.0;
inline float physicsDeltaTime = 1.0f / physicsTicksPerSecond;

} // namespace Physics

#endif // _PHYSICS_H
