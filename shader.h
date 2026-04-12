#pragma once

#include "pch.h"
#include "template.h"

namespace Shader {
typedef struct {
  GLuint program;
  GLuint vertexShader;
  GLuint fragmentShader;
  bool isValid;
} Shader;

// NOTE: From "Game Programming in C++" by Sanjay Madhav
Shader Load(const std::string &vertName, const std::string &fragName);
void Unload(Shader &shader);
void setActive(Shader &shader);
void setMatrixUniform(Shader &shader, const char *name,
                      const Tmpl8::mat4 &matrix);
void setIntUniform(Shader &shader, const char *name, int value);
void setLight(Shader &shader, Tmpl8::mat4 &view);
} // namespace Shader
