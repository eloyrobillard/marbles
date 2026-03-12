#include <SDL_log.h>
#include <fstream>
#include <sstream>

#include "shader.h"

// NOTE: From "Game Programming in C++" by Sanjay Madhav
bool shader_is_compiled(GLuint shader) {
  GLint status;

  // Query the compile status
  __glewGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE) {
    char buffer[512];
    memset(buffer, 0, 512);
    __glewGetShaderInfoLog(shader, 511, nullptr, buffer);

    SDL_Log("GLSL compile failed:\n%s", buffer);
    return false;
  }

  return true;
}

// NOTE: From "Game Programming in C++" by Sanjay Madhav
bool is_valid_shader_program(GLuint shaderProgram) {
  GLint status;

  // Query the program status
  __glewGetProgramiv(shaderProgram, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE) {
    char buffer[512];
    memset(buffer, 0, 512);
    __glewGetProgramInfoLog(shaderProgram, 511, nullptr, buffer);

    SDL_Log("GLSL program failed:\n%s", buffer);
    return false;
  }

  return true;
}

// NOTE: From "Game Programming in C++" by Sanjay Madhav
bool compile_shader(const std::string &filename, GLenum shaderType,
                    GLuint &outShader) {
  std::ifstream shaderFile(filename);
  if (shaderFile.is_open()) {
    // Read all the text into a string
    std::stringstream sstream;
    sstream << shaderFile.rdbuf();
    std::string contents = sstream.str();
    const char *contentsChar = contents.c_str();

    // Create a shader of the specified type
    outShader = __glewCreateShader(shaderType);
    // Set the source characters and try to compile
    __glewShaderSource(outShader, 1, &(contentsChar), nullptr);
    __glewCompileShader(outShader);

    if (!shader_is_compiled(outShader)) {
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
  __glewUseProgram(shaderProgram);
}

// NOTE: From "Game Programming in C++" by Sanjay Madhav
GLProgram load_shader(const std::string &vertName,
                      const std::string &fragName) {
  GLProgram glProgram;

  // Compile vertex and fragment shaders
  if (!compile_shader(vertName, GL_VERTEX_SHADER, glProgram.vertexShader) ||
      !compile_shader(fragName, GL_FRAGMENT_SHADER, glProgram.fragmentShader)) {
    glProgram.isValid = false;
    return glProgram;
  }

  // Now create a shader program that
  // links together the vertex and frag shaders
  glProgram.program = __glewCreateProgram();
  glAttachShader(glProgram.program, glProgram.vertexShader);
  glAttachShader(glProgram.program, glProgram.fragmentShader);
  __glewLinkProgram(glProgram.program);

  // Verify that the program linked successfully
  if (!is_valid_shader_program(glProgram.program)) {
    glProgram.isValid = false;
  }

  return glProgram;
}

void unload_shader(GLProgram &glProgram) {
  __glewDeleteProgram(glProgram.program);
  __glewDeleteShader(glProgram.vertexShader);
  __glewDeleteShader(glProgram.fragmentShader);
}
