#pragma once

#include "maths.hpp"
#include "pch.h"

using Maths::mat4;
// NOTE: From "Game Programming in C++" by Sanjay Madhav
class Shader {
  GLuint program;
  GLuint vertexShader;
  GLuint fragmentShader;

public:
  static optional<Shader> Load(const std::string &vertName,
                               const std::string &fragName);
  void Unload() const;
  void SetLight(mat4 &view) const;
  void SetActive() const { glUseProgram(program); }
  void SetMatrixUniform(const char *name, const mat4 &matrix) const;
  void SetIntUniform(const char *name, int value) const;
  void SetFloatUniform(const char *name, float value) const;
  void SetVec3Uniform(const char *name, const float values[3]) const;
  static void SetVerticesActive(GLuint vertexArray) {
    glBindVertexArray(vertexArray);
  }
};
