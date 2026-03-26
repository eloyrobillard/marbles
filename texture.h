#pragma once

#include "pch.h"

namespace Texture {

class Texture {
public:
  Texture(const string &filename);
  std::string filename;
  GLuint textureID;
  int width;
  int height;
  bool isValid;
};

static std::unordered_map<std::string, Texture *> gAllTextures;

Texture *load(const std::string &filename);
void SetActive(GLuint textureID);
void Unload(GLuint textureID);

} // namespace Texture
