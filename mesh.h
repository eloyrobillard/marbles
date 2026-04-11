#ifndef MESH_H
#define MESH_H

#include "physics.h"
#include "shader.h"
#include "template.h"
#include "texture.h"

using std::vector;
using Tmpl8::quat;
using Tmpl8::vec3;

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
optional<pair<Mesh, Body>> load(const std::string &filename);
void draw(Shader::Shader &shader, Mesh &mesh, Body &body);
void deleteVertexArray(GLuint vertexBuffer, GLuint indexBuffer,
                       GLuint vertexArray);
vector<TriangleCollider> generateTriangleCollidersFromMesh(Mesh &mesh,
                                                           Body &body);

} // namespace Mesh

#endif // MESH_H
