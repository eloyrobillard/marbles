#include "texture.h"
#include "SDL_log.h"
#include "pch.h"

namespace Texture {

Texture load(const std::string &filename) {
  Texture tex{filename, 0, 0, 0};
  int channels = 0;

  unsigned char *image = SOIL_load_image(
      filename.c_str(), &tex.width, &tex.height, &channels, SOIL_LOAD_AUTO);

  if (image == nullptr) {
    SDL_Log("SOIL failed to load image %s: %s", filename.c_str(),
            SOIL_last_result());

    tex.isValid = false;
    return tex;
  }

  int format = GL_RGB;
  if (channels == 4) {
    format = GL_RGBA;
  }

  glGenTextures(1, &tex.textureID);
  glBindTexture(GL_TEXTURE_2D, tex.textureID);

  glTexImage2D(GL_TEXTURE_2D, 0, format, tex.width, tex.height, 0, format,
               GL_UNSIGNED_BYTE, image);

  SOIL_free_image_data(image);

  // Enable linear filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  tex.isValid = true;
  return tex;
}

void SetActive(GLuint textureID) { glBindTexture(GL_TEXTURE_2D, textureID); }
void Unload(GLuint textureID) { glDeleteTextures(1, &textureID); }
} // namespace Texture
