#ifndef MESH_H
#define MESH_H
#include "colliders.hpp"
#include "pch.h"
#include "texture.hpp"

using Maths::mat4;
using Maths::vec3;
using Maths::vec4;

tuple<GLuint, GLuint, GLuint>
createVertexArray(const float *verts, uint numVerts, const uint *indices,
                  uint numIndices, size_t vertSize);
tuple<GLuint, GLuint, GLuint> createVertexArrayVertsOnly(const float *verts,
                                                         uint numVerts,
                                                         const uint *indices,
                                                         uint numIndices,
                                                         size_t vertSize);

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
  // May or may not return a mesh, so it cannot be a constructor
  static optional<pair<Mesh, Body>> Load(const std::string &filename);

  [[nodiscard]] optional<Texture *> lookTextureUp(size_t index) const;
  [[nodiscard]] size_t GetNumIndices() const { return indices.size(); }
  [[nodiscard]] GLuint GetVertexArray() const { return vertexArray; }

  [[nodiscard]] vector<TriangleCollider<PivotBody>>
  generateTriangleCollidersFromMesh(const shared_ptr<PivotBody> &body) const {
    vector<TriangleCollider<PivotBody>> triangles;
    triangles.reserve(idx_triplets.size());

    const mat4 worldTransform = body->getWorldTransform();
    const mat4 rot = mat4::CreateFromQuaternion(body->rotation);

    for (const auto &[i0, i1, i2] : idx_triplets) {
      auto a = vert_coord[i0];
      auto b = vert_coord[i1];
      auto c = vert_coord[i2];

      auto n0 = vert_normal[i0];
      auto n1 = vert_normal[i1];
      auto n2 = vert_normal[i2];

      vec3 average_normal = (n0 + n1 + n2) * (1.0f / 3.0f);
      average_normal.normalize();

      float verts[9] = {a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z};
      uint indices[3] = {0, 1, 2};

      auto [vertexBuffer, indexBuffer, vertexArray] =
          createVertexArrayVertsOnly(verts, 3, indices, 3,
                                     3 /* Position only */);

      triangles.emplace_back(average_normal, a, b, c, body, vertexBuffer,
                             indexBuffer, vertexArray);
    }

    return triangles;
  }

  template <class T>
    requires std::derived_from<T, Body>
  vector<TriangleCollider<T>>
  generateTriangleCollidersFromMesh(const shared_ptr<T> &body) const {
    vector<TriangleCollider<T>> triangles;
    triangles.reserve(idx_triplets.size());

    const mat4 worldTransform = body->getWorldTransform();
    const mat4 rot = mat4::CreateFromQuaternion(body->rotation);

    for (const auto &[i0, i1, i2] : idx_triplets) {
      auto a = vec3(vec4(vert_coord[i0], 1.0f) * worldTransform);
      auto b = vec3(vec4(vert_coord[i1], 1.0f) * worldTransform);
      auto c = vec3(vec4(vert_coord[i2], 1.0f) * worldTransform);

      auto n0 = vert_normal[i0];
      auto n1 = vert_normal[i1];
      auto n2 = vert_normal[i2];

      vec3 average_normal = (n0 + n1 + n2) * (1.0f / 3.0f);
      average_normal = vec3(vec4(average_normal, 1.0f) * rot);
      average_normal.normalize();

      float verts[9] = {a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z};
      uint indices[3] = {0, 1, 2};

      auto [vertexBuffer, indexBuffer, vertexArray] =
          createVertexArrayVertsOnly(verts, 3, indices, 3,
                                     3 /* Position only */);

      triangles.emplace_back(average_normal, a, b, c, body, vertexBuffer,
                             indexBuffer, vertexArray);
    }

    return triangles;
  }
}; // namespace Mesh

#endif // MESH_H
