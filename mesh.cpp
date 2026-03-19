#include "mesh.h"
#include "rapidjson/document.h"
#include "template.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using rapidjson::Value;
using std::cerr;
using std::endl;
using std::vector;
using Tmpl8::vec3;

namespace Mesh {
Mesh load(const std::string &filename) {
  Mesh mesh;

  std::ifstream ifs;
  ifs.open(filename);

  if (!ifs.is_open()) {
    cerr << "Could not open file" << endl;
    return mesh;
  }

  std::stringstream fileStream;
  fileStream << ifs.rdbuf();
  std::string gpmesh = fileStream.str();
  ifs.close();

  rapidjson::Document document;
  document.Parse(gpmesh.c_str());

  if (!document.IsObject()) {
    cerr << "Mesh " << filename << " is not a valid JSON." << endl;
    return mesh;
  }

  std::string shader = document["shader"].GetString();

  Value &vertsJSON = document["vertices"];
  assert(vertsJSON.IsArray() && vertsJSON.Size() > 0);

  std::vector<float> verts;
  verts.reserve(vertsJSON.Size() * 6);

  for (int i = 0; i < vertsJSON.Size(); i++) {
    auto &vert = vertsJSON[i];
    assert(vert.IsArray() && vert.Size() == 16);

    verts.push_back(static_cast<float>(vert[0].GetDouble()));
    verts.push_back(static_cast<float>(vert[1].GetDouble()));
    verts.push_back(static_cast<float>(vert[2].GetDouble()));
    verts.push_back(static_cast<float>(vert[3].GetDouble()));
    verts.push_back(static_cast<float>(vert[4].GetDouble()));
    verts.push_back(static_cast<float>(vert[5].GetDouble()));
  }

  Value &indicesJSON = document["indices"];
  assert(indicesJSON.IsArray() && indicesJSON.Size() > 0);

  std::vector<unsigned int> indices;
  indices.reserve(indicesJSON.Size() * 3);

  for (int i = 0; i < indicesJSON.Size(); i++) {
    auto &index = indicesJSON[i];
    indices.push_back(index[0].GetUint());
    indices.push_back(index[1].GetUint());
    indices.push_back(index[2].GetUint());
  }

  std::cout << indices;
  GLuint vertexArray =
      createVertexArray(static_cast<float *>(verts.data()), verts.size(),
                        static_cast<uint *>(indices.data()), indices.size());

  mesh.verts = verts;
  mesh.indices = indices;
  mesh.vertexArray = vertexArray;
  mesh.isValid = true;

  return mesh;
}

void draw(Shader::Shader &shader, Mesh &mesh) {
  // Set world transform
  Shader::setMatrixUniform(shader, "uWorldTransform", Tmpl8::mat4());

  setVerticesActive(mesh.vertexArray);

  // Draw triangles
  glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
}

GLuint createVertexArray(const float *verts, uint numVerts, uint *indices,
                         uint numIndices) {
  GLuint vertexArray;
  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);

  // Create vertex buffer
  GLuint vertexBuffer = 0;
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(float), verts,
               GL_STATIC_DRAW);

  // Create index buffer
  GLuint indexBuffer = 0;
  glGenBuffers(1, &indexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint), indices,
               GL_STATIC_DRAW);

  // Specify the vertex attributes
  // (For now, assume one vertex format)
  // Position is 3 floats
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
  // Normal is 3 floats
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        reinterpret_cast<void *>(sizeof(float) * 3));

  return vertexArray;
}

void deleteVertexArray(GLuint vertexBuffer, GLuint indexBuffer,
                       GLuint vertexArray) {
  glDeleteBuffers(1, &vertexBuffer);
  glDeleteBuffers(1, &indexBuffer);
  glDeleteBuffers(1, &vertexArray);
}

void setVerticesActive(GLuint vertexArray) { glBindVertexArray(vertexArray); }
} // namespace Mesh
