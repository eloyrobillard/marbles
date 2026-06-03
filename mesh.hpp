#ifndef MESH_H
#define MESH_H
#include "pch.h"
#include "physics.h"
#include "texture.h"

class Mesh {
  vector<float> verts;
  vector<unsigned int> indices;
  GLuint vertexBuffer;
  GLuint indexBuffer;
  GLuint vertexArray;
  vector<Texture *> textures;
  vector<Maths::vec3> vert_coord;
  vector<Maths::vec3> vert_normal;
  vector<std::tuple<uint, uint, uint>> idx_triplets;

  void deleteVertexArray() const;

public:
  static optional<pair<Mesh, Body>> Load(const std::string &filename);
  vector<TriangleCollider>
  generateTriangleCollidersFromMesh(Body &body, float accel,
                                    bool override_impulse,
                                    Maths::vec3 impulse_override) const;
  [[nodiscard]] optional<Texture *> lookTextureUp(size_t index) const;
  [[nodiscard]] size_t GetNumIndices() const { return indices.size(); }
  [[nodiscard]] GLuint GetVertexArray() const { return vertexArray; }

}; // namespace Mesh

#endif // MESH_H
