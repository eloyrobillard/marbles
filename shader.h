#pragma once

#include "pch.h"
#include "template.h"

// NOTE: From "Game Programming in C++" by Sanjay Madhav
class Shader {
  GLuint program;
  GLuint vertexShader;
  GLuint fragmentShader;

public:
  static optional<Shader> Load(const std::string &vertName,
                               const std::string &fragName);
  void Unload() const;
  void setLight(Tmpl8::mat4 &view) const;
  void setActive() const { glUseProgram(program); }
  void setMatrixUniform(const char *name, const Tmpl8::mat4 &matrix) const;
  void setIntUniform(const char *name, int value) const;
  void setFloatUniform(const char *name, float value) const;
  void setVec3Uniform(const char *name, const float values[3]) const;
  static void setVerticesActive(GLuint vertexArray) {
    glBindVertexArray(vertexArray);
  }
};
