#ifndef MESH_H
#define MESH_H
#include "pch.h"
#include "physics.h"
#include "shader.h"
#include "template.h"
#include "texture.h"

class Mesh {
  vector<float> verts;
  vector<unsigned int> indices;
  GLuint vertexBuffer;
  GLuint indexBuffer;
  GLuint vertexArray;
  vector<Texture::Texture *> textures;
  vector<vec3> vert_coord;
  vector<vec3> vert_normal;
  vector<std::tuple<uint, uint, uint>> idx_triplets;

  void deleteVertexArray() const;
  [[nodiscard]] optional<Texture::Texture *> lookTextureUp(size_t index) const;

public:
  void Draw(Shader::Shader &shader, const Body &body) const;
  static optional<pair<Mesh, Body>> Load(const std::string &filename);
  vector<TriangleCollider>
  generateTriangleCollidersFromMesh(Body &body, float accel,
                                    bool override_impulse,
                                    vec3 impulse_override) const;
}; // namespace Mesh

#endif // MESH_H
