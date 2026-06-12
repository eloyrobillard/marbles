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
  vector<vector<TriangleCollider>> mPartition;

  void populate(const TriangleCollider &tc, float min_x, float max_x,
                float min_y, float max_y, float min_z, float max_z);

public:
  SpacePartition(float min_x, float max_x, float min_y, float max_y,
                 float min_z, float max_z, float step)
      : mMinX(min_x), mMaxX(max_x), mMinY(min_y), mMaxY(max_y), mMinZ(min_z),
        mMaxZ(max_z), mStep(step) {
    mNumX = ceil((max_x - min_x) / step);
    mNumY = ceil((max_y - min_y) / step);
    mNumZ = ceil((max_z - min_z) / step);

    mPartition = vector<vector<TriangleCollider>>(mNumX * mNumY * mNumZ);
  }

  SpacePartition(float min_x, float max_x, float min_y, float max_y,
                 float min_z, float max_z, float step,
                 const vector<TriangleCollider> &v)
      : mMinX(min_x), mMaxX(max_x), mMinY(min_y), mMaxY(max_y), mMinZ(min_z),
        mMaxZ(max_z), mStep(step) {
    mNumX = ceil((max_x - min_x) / step);
    mNumY = ceil((max_y - min_y) / step);
    mNumZ = ceil((max_z - min_z) / step);

    mPartition = vector<vector<TriangleCollider>>(mNumX * mNumY * mNumZ);

    populate(v);
  }

  ~SpacePartition() = default;

  void populate(const vector<TriangleCollider> &v);
  [[nodiscard]] vector<TriangleCollider> get_partition(float min_x, float max_x,
                                                       float min_y, float max_y,
                                                       float min_z,
                                                       float max_z) const;
};

// Used for spatial partitioning of static colliders
inline SpacePartition gSpacePartition =
    SpacePartition(4.0f, 150.0f, -40.0f, 80.0f, -100.0f, 10.0f, 1.0f);

class DynamicEntity;

namespace Physics {
bool processStaticCollisions(const vector<TriangleCollider> &triangles,
                             const SphereCollider &sphere, vec3 &velocity);
int processDynamicCollisions(vector<DynamicBody> &des, int idx, int numMarbles,
                             bool joining);

inline float physicsTicksPerSecond = 240.0;
inline float physicsDeltaTime = 1.0f / physicsTicksPerSecond;
} // namespace Physics

#endif // _PHYSICS_H
