#pragma once
#include "glew.h"
#include "template.h"
#include <string>

namespace Shader {
typedef struct {
  GLuint program;
  GLuint vertexShader;
  GLuint fragmentShader;
  bool isValid;
} Shader;

Shader load(const std::string &vertName, const std::string &fragName);
bool compile(const std::string &filename, GLenum shaderType, GLuint &outShader);
bool isCompiled(GLuint shader);
bool shaderProgramIsValid(GLuint shaderProgram);
void unload(Shader &shader);
void setActive(Shader &shader);
void setMatrixUniform(Shader &shader, const char *name,
                      const Tmpl8::mat4 &matrix);
} // namespace Shader
