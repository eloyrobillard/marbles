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
Shader load(const std::string &vertName, const std::string &fragName);
void unload(Shader &shader);
void setActive(Shader &shader);
void setMatrixUniform(Shader &shader, const char *name,
                      const Tmpl8::mat4 &matrix);
} // namespace Shader
