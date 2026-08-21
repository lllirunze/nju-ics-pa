#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  if (src == NULL) return NULL;

  int64_t size = SDL_RWsize(src);
  if (size <= 0 || size > INT32_MAX) {
    if (freesrc) SDL_RWclose(src);
    return NULL;
  }

  uint8_t *data = malloc(size);
  if (data == NULL) {
    if (freesrc) SDL_RWclose(src);
    return NULL;
  }
  SDL_RWseek(src, 0, RW_SEEK_SET);
  size_t nread = SDL_RWread(src, data, 1, size);
  if (freesrc) SDL_RWclose(src);
  if (nread != (size_t)size) {
    free(data);
    return NULL;
  }

  SDL_Surface *surface = STBIMG_LoadFromMemory(data, size);
  free(data);
  return surface;
}

SDL_Surface* IMG_Load(const char *filename) {
  SDL_RWops *src = SDL_RWFromFile(filename, "rb");
  return IMG_Load_RW(src, 1);
}

int IMG_isPNG(SDL_RWops *src) {
  static const uint8_t signature[] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n'};
  uint8_t buf[sizeof(signature)];
  int64_t position = SDL_RWtell(src);
  int ret = SDL_RWread(src, buf, 1, sizeof(buf)) == sizeof(buf) &&
      memcmp(buf, signature, sizeof(buf)) == 0;
  SDL_RWseek(src, position, RW_SEEK_SET);
  return ret;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
