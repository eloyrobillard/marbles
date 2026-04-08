#include <SDL_log.h>
#include <fstream>
#include <sstream>

#include "pch.h"
#include "shader.h"
#include "template.h"

namespace Shader {

bool isCompiled(GLuint shader) {
  // Query the compile status
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE) {
    char buffer[512];
    memset(buffer, 0, 512);
    glGetShaderInfoLog(shader, 511, nullptr, buffer);

    SDL_Log("GLSL compile failed:\n%s", buffer);
    return false;
  }

  return true;
}

bool programIsValid(GLuint shaderProgram) {
  // Query the program status
  GLint status;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);

  if (status != GL_TRUE) {
    char buffer[512];
    memset(buffer, 0, 512);
    glGetProgramInfoLog(shaderProgram, 511, nullptr, buffer);

    SDL_Log("GLSL program failed:\n%s", buffer);
    return false;
  }

  return true;
}

bool compile(const std::string &filename, GLenum shaderType,
             GLuint &outShader) {
  std::ifstream shaderFile(filename);
  if (shaderFile.is_open()) {
    // Read all the text into a string
    std::stringstream sstream;
    sstream << shaderFile.rdbuf();
    std::string contents = sstream.str();
    const char *contentsChar = contents.c_str();

    // Create a shader of the specified type
    outShader = glCreateShader(shaderType);
    // Set the source characters and try to compile
    glShaderSource(outShader, 1, &(contentsChar), nullptr);
    glCompileShader(outShader);

    if (!isCompiled(outShader)) {
      SDL_Log("Failed to compile shader %s", filename.c_str());
      return false;
    }
  } else {
    SDL_Log("Shader file not found: %s", filename.c_str());
    return false;
  }

  return true;
}

void set_shader_program_active(GLuint shaderProgram) {
  glUseProgram(shaderProgram);
}

Shader load(const std::string &vertName, const std::string &fragName) {
  Shader shader;

  // Compile vertex and fragment shaders
  if (!compile(vertName, GL_VERTEX_SHADER, shader.vertexShader) ||
      !compile(fragName, GL_FRAGMENT_SHADER, shader.fragmentShader)) {
    shader.isValid = false;
    return shader;
  }

  // Now create a shader program that
  // links together the vertex and frag shaders
  shader.program = glCreateProgram();
  glAttachShader(shader.program, shader.vertexShader);
  glAttachShader(shader.program, shader.fragmentShader);
  glLinkProgram(shader.program);

  // Verify that the program linked successfully
  shader.isValid = programIsValid(shader.program);

  return shader;
}

void unload(Shader &shader) {
  glDeleteProgram(shader.program);
  glDeleteShader(shader.vertexShader);
  glDeleteShader(shader.fragmentShader);
}

void setActive(Shader &shader) { glUseProgram(shader.program); }

void setMatrixUniform(Shader &shader, const char *name,
                      const Tmpl8::mat4 &matrix) {
  // Find the uniform by this name
  GLuint loc = glGetUniformLocation(shader.program, name);
  // Send the matrix data to the uniform
  glUniformMatrix4fv(loc, 1, GL_TRUE, matrix.cell);
}

void setFloatUniform(Shader &shader, const char *name, const float value) {
  GLuint loc = glGetUniformLocation(shader.program, name);
  glUniform1f(loc, value);
}

void setVec3Uniform(Shader &shader, const char *name, const float values[3]) {
  GLuint loc = glGetUniformLocation(shader.program, name);
  glUniform3fv(loc, 1, values);
}

void setLight(Shader &shader, Tmpl8::mat4 &view) {
  Tmpl8::mat4 camera_pos = view;
  // Camera position is from inverted view
  camera_pos.invert();

  setVec3Uniform(shader, "uCameraPos", view.getTranslation());

  float ambient[3] = {0.1f, 0.1f, 0.1f};
  setVec3Uniform(shader, "uAmbientLight", ambient);

  float direction[3] = {1.0f, 1.0f, -1.0f};
  setVec3Uniform(shader, "uDirLight.direction", direction);

  float diffuse[3] = {1.0f, 1.0f, 1.0f};
  setVec3Uniform(shader, "uDirLight.diffuseColor", diffuse);

  float specular[3] = {1.0f, 1.0f, 1.0f};
  setVec3Uniform(shader, "uDirLight.specularColor", specular);

  // Strength of shine
  float specPower = 32.0f;
  setFloatUniform(shader, "uSpecPower", specPower);
}

} // namespace Shader
