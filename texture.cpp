#include "texture.h"
#include "pch.h"

namespace Texture {

Texture::Texture(const string &fileName) : filename(fileName) {}

optional<Texture *> Load(const std::string &filename) {
  auto *tex = new Texture(filename);
  int channels = 0;

  unsigned char *image = SOIL_load_image(
      filename.c_str(), &tex->width, &tex->height, &channels, SOIL_LOAD_AUTO);

  if (image == nullptr) {
    SDL_Log("SOIL failed to load image %s: %s", filename.c_str(),
            SOIL_last_result());

    return {};
  }

  int format = channels == 4 ? GL_RGBA : GL_RGB;

  glGenTextures(1, &tex->textureID);
  glBindTexture(GL_TEXTURE_2D, tex->textureID);

  glTexImage2D(GL_TEXTURE_2D, 0, format, tex->width, tex->height, 0, format,
               GL_UNSIGNED_BYTE, image);

  SOIL_free_image_data(image);

  // Enable trilinear filtering
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Enable anisotropic filtering
  if (GLEW_EXT_texture_filter_anisotropic) {
    GLfloat max_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                    max_anisotropy);
  }

  return {tex};
}

uint LoadCubemap(vector<string> faces) {
  uint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char *data =
        SOIL_load_image(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, data);
      SOIL_free_image_data(data);
    } else {
      cout << "Cubemap tex failed to load at path: " << faces[i] << endl;
      SOIL_free_image_data(data);
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

void SetActive(GLuint textureID) { glBindTexture(GL_TEXTURE_2D, textureID); }
void Unload(GLuint textureID) { glDeleteTextures(1, &textureID); }
} // namespace Texture
