#ifndef MESH_H
#define MESH_H
#include "pch.h"
#include "physics.h"
#include "shader.h"
#include "template.h"
#include "texture.h"

namespace Mesh {
typedef struct {
  vector<float> verts;
  vector<unsigned int> indices;
  GLuint vertexBuffer;
  GLuint indexBuffer;
  GLuint vertexArray;
  vector<Texture::Texture *> textures;
  vector<vec3> vert_coord;
  vector<vec3> vert_normal;
  vector<std::tuple<uint, uint, uint>> idx_triplets;
} Mesh;

inline void setVerticesActive(GLuint vertexArray) {
  glBindVertexArray(vertexArray);
}
optional<pair<Mesh, Body>> Load(const std::string &filename);
void Draw(Shader::Shader &shader, const Mesh &mesh, const Body &body);
void deleteVertexArray(GLuint vertexBuffer, GLuint indexBuffer,
                       GLuint vertexArray);
vector<TriangleCollider>
generateTriangleCollidersFromMesh(Mesh &mesh, Body &body, float accel,
                                  bool override_impulse, vec3 impulse_override);

} // namespace Mesh

#endif // MESH_H
