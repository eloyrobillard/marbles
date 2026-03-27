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
  GLuint vertexArray;
  vector<Texture::Texture *> textures;
  bool isValid;
} Mesh;

std::pair<Mesh, Body> load(const std::string &filename);
void draw(Shader::Shader &shader, Mesh &mesh, Body &rb);
void deleteVertexArray(GLuint vertexBuffer, GLuint indexBuffer,
                       GLuint vertexArray);
} // namespace Mesh

#endif // MESH_H
