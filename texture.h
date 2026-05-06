#pragma once

#include "pch.h"

class Texture {
  std::string filename;
  GLuint textureID;
  int width;
  int height;

public:
  Texture(string filename);
  static optional<Texture *> Load(const string &filename);
  static uint LoadCubemap(vector<string> faces);
  void SetActive() const { glBindTexture(GL_TEXTURE_2D, textureID); }
  void Unload() { glDeleteTextures(1, &textureID); }
};

static std::unordered_map<std::string, Texture *> gAllTextures;
