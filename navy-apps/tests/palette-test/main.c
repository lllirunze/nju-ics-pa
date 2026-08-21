#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
  SDL_Surface *src = SDL_CreateRGBSurface(0, 2, 2, 8, 0, 0, 0, 0);
  SDL_Surface *dst = SDL_CreateRGBSurface(0, 4, 4, 8, 0, 0, 0, 0);
  SDL_Color palette[4] = {
    {.r = 0x00, .g = 0x00, .b = 0x00},
    {.r = 0xff, .g = 0x00, .b = 0x00},
    {.r = 0x00, .g = 0xff, .b = 0x00},
    {.r = 0x00, .g = 0x00, .b = 0xff},
  };
  SDL_SetPalette(src, SDL_LOGPAL, palette, 0, 4);
  SDL_SetPalette(dst, SDL_LOGPAL, palette, 0, 4);

  uint8_t pixels[] = {1, 2, 3, 0};
  memcpy(src->pixels, pixels, sizeof(pixels));
  SDL_Rect rect = {.x = 0, .y = 0, .w = 4, .h = 4};
  SDL_SoftStretch(src, NULL, dst, &rect);

  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      assert(dst->pixels[y * dst->pitch + x] == pixels[(y / 2) * 2 + x / 2]);
    }
  }

  SDL_Color white = {.r = 0xff, .g = 0xff, .b = 0xff};
  SDL_SetPalette(dst, SDL_LOGPAL, &white, 2, 1);
  assert(dst->format->palette->colors[2].r == 0xff);
  assert(dst->format->palette->colors[2].g == 0xff);
  assert(dst->format->palette->colors[2].b == 0xff);
  puts("8-bit palette PASS");

  SDL_FreeSurface(dst);
  SDL_FreeSurface(src);
  return 0;
}
