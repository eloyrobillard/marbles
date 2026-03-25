#pragma once

#include "shader.h"
#include <string>
#include <unordered_map>
namespace Texture {

struct Texture {
  std::string filename;
  GLuint textureID;
  int width;
  int height;
  bool isValid;
};

static std::unordered_map<std::string, Texture *> gAllTextures;

Texture load(const std::string &filename);
void SetActive(GLuint textureID);
void Unload(GLuint textureID);

} // namespace Texture
