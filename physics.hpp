#ifndef _PHYSICS_H
#define _PHYSICS_H
#pragma once

#include "colliders.hpp"
#include "entities.hpp"
#include "maths.hpp"
#include "pch.h"
#include "spacePartition.hpp"

using Maths::mat4;
using Maths::quat;
using Maths::vec3;

const vec3 gGravity = vec3(0.0f, 0.0f, -9.81f);

// Used for spatial partitioning of static colliders
inline SpacePartition<Mesh> gSpacePartition =
    SpacePartition<Mesh>(4.0f, 150.0f, -40.0f, 80.0f, -100.0f, 10.0f, 10.0f);

class DynamicEntity;

namespace Physics {
bool processStaticCollisions(const vector<TriangleCollider> &triangles,
                             const Mesh &mesh, const SphereCollider &sphere,
                             vec3 &velocity);
int processDynamicCollisions(vector<DynamicBody> &des, int idx, int numMarbles,
                             bool joining);

inline float physicsTicksPerSecond = 240.0;
inline float physicsDeltaTime = 1.0f / physicsTicksPerSecond;
} // namespace Physics

#endif // _PHYSICS_H
