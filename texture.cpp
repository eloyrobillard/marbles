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

void SetActive(GLuint textureID) { glBindTexture(GL_TEXTURE_2D, textureID); }
void Unload(GLuint textureID) { glDeleteTextures(1, &textureID); }
} // namespace Texture
