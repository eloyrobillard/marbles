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
using std::cout;
using std::endl;
using std::vector;
using Tmpl8::mat4;
using Tmpl8::vec3;
using Tmpl8::vec4;

namespace Mesh {

GLuint createVertexArray(const float *verts, uint numVerts, const uint *indices,
                         uint numIndices, size_t sizeVert) {

  GLuint vertexArray;
  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);

  // Create vertex buffer
  GLuint vertexBuffer = 0;
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, numVerts * sizeVert * sizeof(float), verts,
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeVert * sizeof(float),
                        nullptr);
  // Normal is 3 floats
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeVert * sizeof(float),
                        reinterpret_cast<void *>(sizeof(float) * 3));
  // Texture coordinates is 2 floats
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, sizeVert * sizeof(float),
      reinterpret_cast<void *>(sizeof(float) * (sizeVert - 2)));

  return vertexArray;
}

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

  Value &scaleJSON = document["scale"];
  if (scaleJSON.IsArray() && scaleJSON.Size() == 3) {
    mesh.scale.x = static_cast<float>(scaleJSON[0].GetDouble());
    mesh.scale.y = static_cast<float>(scaleJSON[1].GetDouble());
    mesh.scale.z = static_cast<float>(scaleJSON[2].GetDouble());
  }

  Value &rotJSON = document["rotationQuaternion"];
  if (rotJSON.IsArray() && rotJSON.Size() == 4) {
    mesh.rotation.x = static_cast<float>(rotJSON[0].GetDouble());
    mesh.rotation.y = static_cast<float>(rotJSON[1].GetDouble());
    mesh.rotation.z = static_cast<float>(rotJSON[2].GetDouble());
    mesh.rotation.w = static_cast<float>(rotJSON[3].GetDouble());
  }

  Value &translationJSON = document["translation"];
  if (translationJSON.IsArray() && translationJSON.Size() == 3) {
    mesh.translation.x = static_cast<float>(translationJSON[0].GetDouble());
    mesh.translation.y = static_cast<float>(translationJSON[1].GetDouble());
    mesh.translation.z = static_cast<float>(translationJSON[2].GetDouble());
  }

  Value &vertsJSON = document["vertices"];
  assert(vertsJSON.IsArray() && vertsJSON.Size() > 0);

  size_t sizeVert = 8;

  std::vector<float> verts;
  verts.reserve(vertsJSON.Size() * sizeVert);

  for (int i = 0; i < vertsJSON.Size(); i++) {
    auto &vert = vertsJSON[i];
    assert(vert.IsArray() && vert.Size() == 16);

    verts.push_back(static_cast<float>(vert[0].GetDouble()));
    verts.push_back(static_cast<float>(vert[1].GetDouble()));
    verts.push_back(static_cast<float>(vert[2].GetDouble()));
    verts.push_back(static_cast<float>(vert[3].GetDouble()));
    verts.push_back(static_cast<float>(vert[4].GetDouble()));
    verts.push_back(static_cast<float>(vert[5].GetDouble()));
    verts.push_back(static_cast<float>(vert[14].GetDouble()));
    verts.push_back(static_cast<float>(vert[15].GetDouble()));
  }

  Value &indicesJSON = document["indices"];
  assert(indicesJSON.IsArray() && indicesJSON.Size() > 0);

  std::vector<unsigned int> indices;
  indices.reserve(indicesJSON.Size() * 3);

  for (int i = 0; i < indicesJSON.Size(); i++) {
    auto &index = indicesJSON[i];
    indices.emplace_back(index[0].GetUint());
    indices.emplace_back(index[1].GetUint());
    indices.emplace_back(index[2].GetUint());
  }

  GLuint vertexArray = createVertexArray(
      verts.data(), vertsJSON.Size(), indices.data(), indices.size(), sizeVert);

  mesh.verts = verts;
  mesh.indices = indices;
  mesh.vertexArray = vertexArray;
  mesh.isValid = true;

  return mesh;
}

void setVerticesActive(GLuint vertexArray) { glBindVertexArray(vertexArray); }

void draw(Shader::Shader &shader, Mesh &mesh) {
  using Tmpl8::mat4;

  // Set world transform
  mat4 scale = mat4::CreateScale(mesh.scale);

  mat4 rotation = mat4::CreateFromQuaternion(mesh.rotation);

  mat4 translation = mat4::CreateTranslation(mesh.translation);

  mat4 worldTransform = scale;
  worldTransform *= rotation;
  worldTransform *= translation;

  Shader::setMatrixUniform(shader, "uWorldTransform", worldTransform);

  setVerticesActive(mesh.vertexArray);

  // Draw triangles
  glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
}

void deleteVertexArray(GLuint vertexBuffer, GLuint indexBuffer,
                       GLuint vertexArray) {
  glDeleteBuffers(1, &vertexBuffer);
  glDeleteBuffers(1, &indexBuffer);
  glDeleteBuffers(1, &vertexArray);
}

} // namespace Mesh
