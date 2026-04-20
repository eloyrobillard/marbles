#include "mesh.h"
#include "SDL_log.h"
#include "pch.h"
#include "physics.h"
#include "rapidjson/document.h"
#include "template.h"
#include "texture.h"

using Tmpl8::mat4;
using Tmpl8::vec3;
using Tmpl8::vec4;

namespace Mesh {

tuple<GLuint, GLuint, GLuint>
createVertexArray(const float *verts, uint numVerts, const uint *indices,
                  uint numIndices, size_t vertSize) {

  GLuint vertexArray;
  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);

  // Create vertex buffer
  GLuint vertexBuffer = 0;
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, numVerts * vertSize * sizeof(float), verts,
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertSize * sizeof(float),
                        nullptr);
  // Normal is 3 floats
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertSize * sizeof(float),
                        reinterpret_cast<void *>(sizeof(float) * 3));
  // Texture coordinates is 2 floats
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, vertSize * sizeof(float),
      reinterpret_cast<void *>(sizeof(float) * (vertSize - 2)));

  return {vertexBuffer, indexBuffer, vertexArray};
}

tuple<GLuint, GLuint, GLuint> createVertexArrayVertsOnly(const float *verts,
                                                         uint numVerts,
                                                         const uint *indices,
                                                         uint numIndices,
                                                         size_t vertSize) {

  GLuint vertexArray;
  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);

  // Create vertex buffer
  GLuint vertexBuffer = 0;
  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, numVerts * vertSize * sizeof(float), verts,
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertSize * sizeof(float),
                        nullptr);

  return {vertexBuffer, indexBuffer, vertexArray};
}

optional<Texture::Texture *> GetTexture(Mesh &mesh,
                                        const std::string &fileName) {
  auto iter = Texture::gAllTextures.find(fileName);

  if (iter != Texture::gAllTextures.end()) {
    return {iter->second};
  } else {
    auto maybe_tex = Texture::Load(fileName);

    if (maybe_tex.has_value()) {
      Texture::gAllTextures.emplace(fileName, maybe_tex.value());
    }

    return maybe_tex;
  }
}

optional<pair<Mesh, Body>> Load(const std::string &filename) {
  Mesh mesh{};
  Body body{};

  std::ifstream ifs;
  ifs.open(filename);

  if (!ifs.is_open()) {
    cerr << "Could not open file" << endl;
    return {};
  }

  std::stringstream fileStream;
  fileStream << ifs.rdbuf();
  std::string gpmesh = fileStream.str();

  rapidjson::Document document;
  document.Parse(gpmesh.c_str());

  if (!document.IsObject()) {
    cerr << "Mesh " << filename << " is not a valid JSON." << endl;
    return {};
  }

  std::string shader = document["shader"].GetString();

  // Load textures
  const Value &textures = document["textures"];
  if (!textures.IsArray() || textures.Size() < 1) {
    SDL_Log("Mesh %s has no textures, using the default one", filename.c_str());

    mesh.textures.emplace_back(GetTexture(mesh, "Assets/Default.png").value());
  } else {
    for (rapidjson::SizeType i = 0; i < textures.Size(); i++) {
      // Is this texture already loaded?
      std::string texName = textures[i].GetString();

      optional<Texture::Texture *> maybe_tex = GetTexture(mesh, texName);
      if (!maybe_tex.has_value()) {
        // Try loading the texture
        maybe_tex = GetTexture(mesh, texName);
        if (!maybe_tex.has_value()) {
          // If it's still null, just use the default texture
          maybe_tex = GetTexture(mesh, "Assets/Default.png");
        }
      }

      if (maybe_tex.has_value())
        mesh.textures.emplace_back(maybe_tex.value());
    }
  }

  Value &scaleJSON = document["scale"];
  if (!scaleJSON.IsArray() && scaleJSON.Size() != 3) {
    SDL_Log("Mesh %s should have scale info", filename.c_str());

    return {};
  }

  body.scale.x = static_cast<float>(scaleJSON[0].GetDouble());
  body.scale.y = static_cast<float>(scaleJSON[1].GetDouble());
  body.scale.z = static_cast<float>(scaleJSON[2].GetDouble());

  Value &rotJSON = document["rotationQuaternion"];
  if (!rotJSON.IsArray() && rotJSON.Size() != 4) {
    SDL_Log("Mesh %s should have rotation info", filename.c_str());

    return {};
  }

  body.rotation.x = static_cast<float>(rotJSON[0].GetDouble());
  body.rotation.y = static_cast<float>(rotJSON[1].GetDouble());
  body.rotation.z = static_cast<float>(rotJSON[2].GetDouble());
  body.rotation.w = static_cast<float>(rotJSON[3].GetDouble());

  Value &translationJSON = document["translation"];
  if (!translationJSON.IsArray() && translationJSON.Size() != 3) {
    SDL_Log("Mesh %s should have translation coordinates", filename.c_str());

    return {};
  }

  body.position.x = static_cast<float>(translationJSON[0].GetDouble());
  body.position.y = static_cast<float>(translationJSON[1].GetDouble());
  body.position.z = static_cast<float>(translationJSON[2].GetDouble());

  Value &vertsJSON = document["vertices"];
  if (!vertsJSON.IsArray() && vertsJSON.Size() <= 0) {
    SDL_Log("Mesh %s should have vertices", filename.c_str());

    return {};
  }

  size_t vertSize = 8;

  vector<float> verts;
  verts.reserve(vertsJSON.Size() * vertSize);
  vector<vec3> vert_coord;
  vector<vec3> vert_norm;
  vert_coord.reserve(vertsJSON.Size());
  vert_norm.reserve(vertsJSON.Size());

  for (int i = 0; i < vertsJSON.Size(); i++) {
    auto &vert = vertsJSON[i];
    if (!vert.IsArray() || vert.Size() != 16) {
      SDL_Log("Vertex of the wrong size (size: %d, idx: %d). Skipping...",
              vert.Size(), i);
      continue;
    }

    float coord0 = vert[0].GetDouble();
    float coord1 = vert[1].GetDouble();
    float coord2 = vert[2].GetDouble();

    vert_coord.emplace_back(coord0, coord1, coord2);

    float norm0 = vert[3].GetDouble();
    float norm1 = vert[4].GetDouble();
    float norm2 = vert[5].GetDouble();

    vert_norm.emplace_back(norm0, norm1, norm2);

    verts.push_back(static_cast<float>(coord0));
    verts.push_back(static_cast<float>(coord1));
    verts.push_back(static_cast<float>(coord2));
    verts.push_back(static_cast<float>(norm0));
    verts.push_back(static_cast<float>(norm1));
    verts.push_back(static_cast<float>(norm2));
    verts.push_back(static_cast<float>(vert[14].GetDouble()));
    verts.push_back(static_cast<float>(vert[15].GetDouble()));
  }

  Value &indicesJSON = document["indices"];
  if (!indicesJSON.IsArray() && indicesJSON.Size() <= 0) {
    SDL_Log("Mesh %s should have vertex indices", filename.c_str());

    return {};
  }

  std::vector<uint> indices;
  indices.reserve(indicesJSON.Size() * 3);
  std::vector<std::tuple<uint, uint, uint>> idx_triplets;
  idx_triplets.reserve(indicesJSON.Size());

  for (int i = 0; i < indicesJSON.Size(); i++) {
    auto &index = indicesJSON[i];
    if (!index.IsArray() || index.Size() != 3) {
      SDL_Log("Vertex of the wrong size (size: %d, idx: %d). Skipping...",
              index.Size(), i);
      continue;
    }

    uint a = index[0].GetUint();
    uint b = index[1].GetUint();
    uint c = index[2].GetUint();

    indices.emplace_back(a);
    indices.emplace_back(b);
    indices.emplace_back(c);

    idx_triplets.emplace_back(a, b, c);
  }

  auto [vertexBuffer, indexBuffer, vertexArray] =
      createVertexArray(verts.data(), verts.size() / vertSize, indices.data(),
                        indices.size(), vertSize);

  mesh.verts = verts;
  mesh.indices = indices;
  mesh.vertexBuffer = vertexBuffer;
  mesh.indexBuffer = indexBuffer;
  mesh.vertexArray = vertexArray;
  mesh.vert_coord = vert_coord;
  mesh.vert_normal = vert_norm;
  mesh.idx_triplets = idx_triplets;

  return {{mesh, body}};
}

optional<Texture::Texture *> lookTextureUp(const Mesh &mesh, size_t index) {
  if (index < mesh.textures.size())
    return {mesh.textures[index]};
  else
    return {};
}

mat4 getWorldTransform(const Body &body) {
  mat4 scale = mat4::CreateScale(body.scale);
  mat4 rotation = mat4::CreateFromQuaternion(body.rotation);
  mat4 translation = mat4::CreateTranslation(body.position);

  return scale * rotation * translation;
}

void Draw(Shader::Shader &shader, const Mesh &mesh, const Body &body) {
  // Set world transform
  mat4 worldTransform = getWorldTransform(body);

  Shader::setMatrixUniform(shader, "uWorldTransform", worldTransform);

  auto maybe_tex = lookTextureUp(mesh, 0);
  if (maybe_tex.has_value())
    Texture::SetActive(maybe_tex.value()->textureID);

  setVerticesActive(mesh.vertexArray);

  // Draw triangles
  glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);

  GLenum err_code = glGetError();
  while (GL_NO_ERROR != err_code) {
    printf("OpenGL Error @ %s: %i", "mesh draw", err_code);
    err_code = glGetError();
  }
}

void deleteVertexArray(GLuint vertexBuffer, GLuint indexBuffer,
                       GLuint vertexArray) {
  glDeleteBuffers(1, &vertexBuffer);
  glDeleteBuffers(1, &indexBuffer);
  glDeleteBuffers(1, &vertexArray);
}

vector<TriangleCollider>
generateTriangleCollidersFromMesh(Mesh &mesh, Body &body, float accel,
                                  bool override_impulse,
                                  vec3 impulse_override) {
  vector<TriangleCollider> triangles;
  triangles.reserve(mesh.idx_triplets.size());

  const mat4 worldTransform = getWorldTransform(body);

  for (const auto &[i0, i1, i2] : mesh.idx_triplets) {
    auto a = vec3(vec4(mesh.vert_coord[i0], 1.0f) * worldTransform);
    auto b = vec3(vec4(mesh.vert_coord[i1], 1.0f) * worldTransform);
    auto c = vec3(vec4(mesh.vert_coord[i2], 1.0f) * worldTransform);

    auto n0 = mesh.vert_normal[i0];
    auto n1 = mesh.vert_normal[i1];
    auto n2 = mesh.vert_normal[i2];

    mat4 rot = mat4::CreateFromQuaternion(body.rotation);

    vec3 average_normal = (n0 + n1 + n2) * (1.0f / 3.0f);
    average_normal = vec4(average_normal, 1.0f) * rot;

    vec3 impov = vec4(impulse_override, 1.0f) * rot;

    float verts[9] = {a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z};
    uint indices[3] = {0, 1, 2};

    auto [vertexBuffer, indexBuffer, vertexArray] =
        createVertexArrayVertsOnly(verts, 3, indices, 3, 3 /* Position only */);

    // NOTE: leaving constructor so clangd reports type errors
    triangles.emplace_back(
        TriangleCollider(average_normal, a, b, c, accel, override_impulse,
                         impov, vertexBuffer, indexBuffer, vertexArray));
  }

  return triangles;
}

} // namespace Mesh
