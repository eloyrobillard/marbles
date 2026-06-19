#ifndef MESH_H
#define MESH_H
#include "colliders.hpp"
#include "pch.h"
#include "spacePartition.hpp"
#include "texture.hpp"

struct Mesh {
  float minX = 0.0f;
  float maxX = 0.0f;
  float minY = 0.0f;
  float maxY = 0.0f;
  float minZ = 0.0f;
  float maxZ = 0.0f;

  vector<float> verts;
  vector<unsigned int> indices;
  GLuint vertexBuffer;
  GLuint indexBuffer;
  GLuint vertexArray;
  vector<Texture *> textures;
  vector<Maths::vec3> vert_coord;
  vector<Maths::vec3> vert_normal;
  vector<tuple<uint, uint, uint>> idx_triplets;
  SpacePartition<TriangleCollider> spacePartition;

  float accel;
  bool overrideSpeed;
  vec3 speedOverride;
  bool overrideImpulse;
  vec3 impulseOverride;

  void deleteVertexArray() const;

public:
  // May or may not return a mesh, so it cannot be a constructor
  static optional<pair<Mesh, Body>> Load(const std::string &filename);
  void generateSpacePartition(Body &body);
  [[nodiscard]] optional<Texture *> lookTextureUp(size_t index) const;
  [[nodiscard]] size_t GetNumIndices() const { return indices.size(); }
  [[nodiscard]] GLuint GetVertexArray() const { return vertexArray; }
  [[nodiscard]] tuple<float, float, float, float, float, float>
  GetBounds() const {
    return {minX, maxX, minY, maxY, minZ, maxZ};
  }

}; // namespace Mesh

#endif // MESH_H
