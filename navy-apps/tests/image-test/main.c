#include <SDL.h>
#include <SDL_image.h>
#include <assert.h>
#include <stdio.h>

int main() {
  SDL_Init(0);
  SDL_Surface *image = IMG_Load("/share/pictures/projectn.bmp");
  assert(image != NULL);
  assert(image->w > 0 && image->h > 0);
  assert(image->format->BitsPerPixel == 32);
  printf("IMG_Load PASS: %dx%d\n", image->w, image->h);
  SDL_FreeSurface(image);
  SDL_Quit();
  return 0;
}
