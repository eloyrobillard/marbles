#include "shader.hpp"

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

optional<Shader> Shader::Load(const std::string &vertName,
                              const std::string &fragName) {
  Shader shader{};

  // Compile vertex and fragment shaders
  if (!compile(vertName, GL_VERTEX_SHADER, shader.vertexShader) ||
      !compile(fragName, GL_FRAGMENT_SHADER, shader.fragmentShader)) {
    return {};
  }

  // Now create a shader program that
  // links together the vertex and frag shaders
  shader.program = glCreateProgram();
  glAttachShader(shader.program, shader.vertexShader);
  glAttachShader(shader.program, shader.fragmentShader);
  glLinkProgram(shader.program);

  // Verify that the program linked successfully
  if (programIsValid(shader.program)) {
    return {shader};
  }

  return {};
}

void Shader::Unload() const {
  glDeleteProgram(program);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void Shader::SetMatrixUniform(const char *name,
                              const Maths::mat4 &matrix) const {
  // Find the uniform by this name
  GLuint loc = glGetUniformLocation(program, name);
  // Send the matrix data to the uniform
  glUniformMatrix4fv(static_cast<GLint>(loc), 1, GL_TRUE, matrix.cell);
}

void Shader::SetBoolUniform(const char *name, bool value) const {
  GLuint loc = glGetUniformLocation(program, name);
  glUniform1i(static_cast<GLint>(loc), value);
}

void Shader::SetIntUniform(const char *name, int value) const {
  GLuint loc = glGetUniformLocation(program, name);
  glUniform1i(static_cast<GLint>(loc), value);
}

void Shader::SetFloatUniform(const char *name, const float value) const {
  GLuint loc = glGetUniformLocation(program, name);
  glUniform1f(static_cast<GLint>(loc), value);
}

void Shader::SetVec3Uniform(const char *name, const float values[3]) const {
  GLuint loc = glGetUniformLocation(program, name);
  glUniform3fv(static_cast<GLint>(loc), 1, values);
}

void Shader::SetVec3Uniform(const char *name, const Maths::vec3 &v) const {
  GLuint loc = glGetUniformLocation(program, name);
  float values[3] = {v.x, v.y, v.z};
  glUniform3fv(static_cast<GLint>(loc), 1, values);
}

void Shader::SetLight(mat4 &view) const {
  mat4 camera_pos = view;
  // Camera position is from inverted view
  camera_pos.invert();

  SetVec3Uniform("uCameraPos", view.getTranslation());

  float ambient[3] = {0.3f, 0.3f, 0.3f};
  SetVec3Uniform("uAmbientLight", ambient);

  float diffuse[3] = {1.0f, 1.0f, 1.0f};
  SetVec3Uniform("uDirLight.diffuseColor", diffuse);

  float specular[3] = {1.0f, 1.0f, 1.0f};
  SetVec3Uniform("uDirLight.specularColor", specular);

  // Strength of shine
  float specPower = 32.0f;
  SetFloatUniform("uSpecPower", specPower);
}
