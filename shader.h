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

// NOTE: From "Game Programming in C++" by Sanjay Madhav
Shader load(const std::string &vertName, const std::string &fragName);
// NOTE: From "Game Programming in C++" by Sanjay Madhav
bool compile(const std::string &filename, GLenum shaderType, GLuint &outShader);
// NOTE: From "Game Programming in C++" by Sanjay Madhav
bool isCompiled(GLuint shader);
// NOTE: From "Game Programming in C++" by Sanjay Madhav
bool programIsValid(GLuint shaderProgram);
// NOTE: From "Game Programming in C++" by Sanjay Madhav
void unload(Shader &shader);
// NOTE: From "Game Programming in C++" by Sanjay Madhav
void setActive(Shader &shader);
// NOTE: From "Game Programming in C++" by Sanjay Madhav
void setMatrixUniform(Shader &shader, const char *name,
                      const Tmpl8::mat4 &matrix);
} // namespace Shader
