#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  /**
   * make decoded pixels into `Surface` type
   * 1. open file using `libc` and get the `size` of file
   * 2. init `buf[size]`
   * 3. write the whole file to `buf`
   * 4. call STBIMG_LoadFromMemory() (use `buf` and `size` as parameters), which returns `*SDL_Surface`
   * 5. close file, free `buf`
   * 6. return `*SDL_Surface`
   */
  FILE *fp;

  fp = fopen(filename, "rb");
  if (fp == NULL) {
    printf("image %s isn't found.\n", filename);
    return NULL;
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *buf = (char *)SDL_malloc(size);
  assert(fread(buf, size, 1, fp));
  SDL_Surface *surface = STBIMG_LoadFromMemory(buf, size);
  SDL_free(buf);
  return surface;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
