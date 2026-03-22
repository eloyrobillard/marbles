#ifndef MESH_H
#define MESH_H

#include <vector>

#include "shader.h"
#include "template.h"

using std::vector;
using Tmpl8::quat;
using Tmpl8::vec3;

namespace Mesh {
typedef struct {
  quat rot;
  vec3 scale;
  vec3 translation;
  vector<float> verts;
  vector<unsigned int> indices;
  GLuint vertexArray;
  bool isValid;
} Mesh;

Mesh load(const std::string &filename);
void draw(Shader::Shader &shader, Mesh &mesh);
void deleteVertexArray(GLuint vertexBuffer, GLuint indexBuffer,
                       GLuint vertexArray);
} // namespace Mesh

#endif // MESH_H
