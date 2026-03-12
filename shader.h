#pragma once
#include "glew.h"
#include <string>

typedef struct {
  GLuint program;
  GLuint vertexShader;
  GLuint fragmentShader;
  bool isValid;
} GLProgram;

GLProgram load_shader(const std::string &vertName, const std::string &fragName);
bool compile_shader(const std::string &filename, GLenum shaderType,
                    GLuint &outShader);
bool shader_is_compiled(GLuint shader);
bool is_valid_shader_program();
void unload_shader(GLProgram &glProgram);
