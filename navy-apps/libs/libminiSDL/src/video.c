#include <NDL.h>
#include <sdl-video.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static uint32_t pixel_to_rgb(uint32_t pixel, SDL_PixelFormat *src) {
  uint8_t r = (pixel & src->Rmask) >> src->Rshift;
  uint8_t g = (pixel & src->Gmask) >> src->Gshift;
  uint8_t b = (pixel & src->Bmask) >> src->Bshift;
  return (r << 16) | (g << 8) | b;
}

static uint32_t convert_pixel(uint32_t pixel, SDL_PixelFormat *src, SDL_PixelFormat *dst) {
  uint8_t r = (pixel & src->Rmask) >> src->Rshift;
  uint8_t g = (pixel & src->Gmask) >> src->Gshift;
  uint8_t b = (pixel & src->Bmask) >> src->Bshift;
  uint8_t a = src->Amask == 0 ? 0xff : (pixel & src->Amask) >> src->Ashift;
  return SDL_MapRGBA(dst, r, g, b, a);
}

void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(dst && src);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);

  int src_x = srcrect == NULL ? 0 : srcrect->x;
  int src_y = srcrect == NULL ? 0 : srcrect->y;
  int width = srcrect == NULL ? src->w : srcrect->w;
  int height = srcrect == NULL ? src->h : srcrect->h;
  int dst_x = dstrect == NULL ? 0 : dstrect->x;
  int dst_y = dstrect == NULL ? 0 : dstrect->y;
  int bytes_per_pixel = src->format->BytesPerPixel;

  assert(src_x >= 0 && src_y >= 0 && width >= 0 && height >= 0);
  assert(src_x + width <= src->w && src_y + height <= src->h);

  if (dst_x < 0) {
    src_x -= dst_x;
    width += dst_x;
    dst_x = 0;
  }
  if (dst_y < 0) {
    src_y -= dst_y;
    height += dst_y;
    dst_y = 0;
  }
  if (dst_x + width > dst->w) width = dst->w - dst_x;
  if (dst_y + height > dst->h) height = dst->h - dst_y;
  if (width <= 0 || height <= 0) return;

  for (int row = 0; row < height; row++) {
    uint8_t *src_row = src->pixels + (src_y + row) * src->pitch + src_x * bytes_per_pixel;
    uint8_t *dst_row = dst->pixels + (dst_y + row) * dst->pitch + dst_x * bytes_per_pixel;
    if (bytes_per_pixel == 1 ||
        (src->format->Rmask == dst->format->Rmask &&
         src->format->Gmask == dst->format->Gmask &&
         src->format->Bmask == dst->format->Bmask &&
         src->format->Amask == dst->format->Amask)) {
      memcpy(dst_row, src_row, width * bytes_per_pixel);
    } else {
      assert(bytes_per_pixel == 4);
      uint32_t *src_pixels = (uint32_t *)src_row;
      uint32_t *dst_pixels = (uint32_t *)dst_row;
      for (int col = 0; col < width; col++) {
        dst_pixels[col] = convert_pixel(src_pixels[col], src->format, dst->format);
      }
    }
  }
}

void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color) {
  assert(dst);
  int x = dstrect == NULL ? 0 : dstrect->x;
  int y = dstrect == NULL ? 0 : dstrect->y;
  int width = dstrect == NULL ? dst->w : dstrect->w;
  int height = dstrect == NULL ? dst->h : dstrect->h;

  assert(x >= 0 && y >= 0 && width >= 0 && height >= 0);
  assert(x + width <= dst->w && y + height <= dst->h);
  if (dst->format->BytesPerPixel == 1) {
    for (int row = 0; row < height; row++) {
      memset(dst->pixels + (y + row) * dst->pitch + x, color, width);
    }
  } else {
    assert(dst->format->BytesPerPixel == 4);
    for (int row = 0; row < height; row++) {
      uint32_t *pixels = (uint32_t *)(dst->pixels + (y + row) * dst->pitch) + x;
      for (int col = 0; col < width; col++) pixels[col] = color;
    }
  }
}

void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h) {
  assert(s);
  if (!(s->flags & SDL_HWSURFACE)) return;
  if (w == 0) w = s->w;
  if (h == 0) h = s->h;
  assert(x >= 0 && y >= 0 && w >= 0 && h >= 0);
  assert(x + w <= s->w && y + h <= s->h);

  uint32_t *row_pixels = malloc(sizeof(uint32_t) * w);
  assert(row_pixels);
  for (int row = 0; row < h; row++) {
    if (s->format->BytesPerPixel == 4) {
      uint32_t *pixels = (uint32_t *)(s->pixels + (y + row) * s->pitch) + x;
      for (int col = 0; col < w; col++) {
        row_pixels[col] = pixel_to_rgb(pixels[col], s->format);
      }
    } else {
      assert(s->format->BytesPerPixel == 1 && s->format->palette != NULL);
      uint8_t *indices = s->pixels + (y + row) * s->pitch + x;
      for (int col = 0; col < w; col++) {
        SDL_Color c = s->format->palette->colors[indices[col]];
        row_pixels[col] = (c.r << 16) | (c.g << 8) | c.b;
      }
    }
    NDL_DrawRect(row_pixels, x, y + row, w, 1);
  }
  free(row_pixels);
}

// APIs below are already implemented.

static inline int maskToShift(uint32_t mask) {
  switch (mask) {
    case 0x000000ff: return 0;
    case 0x0000ff00: return 8;
    case 0x00ff0000: return 16;
    case 0xff000000: return 24;
    case 0x00000000: return 24; // hack
    default: assert(0);
  }
}

SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
    uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  assert(depth == 8 || depth == 32);
  SDL_Surface *s = malloc(sizeof(SDL_Surface));
  assert(s);
  s->flags = flags;
  s->format = malloc(sizeof(SDL_PixelFormat));
  assert(s->format);
  memset(s->format, 0, sizeof(SDL_PixelFormat));
  if (depth == 8) {
    s->format->palette = malloc(sizeof(SDL_Palette));
    assert(s->format->palette);
    s->format->palette->colors = malloc(sizeof(SDL_Color) * 256);
    assert(s->format->palette->colors);
    memset(s->format->palette->colors, 0, sizeof(SDL_Color) * 256);
    s->format->palette->ncolors = 256;
  } else {
    s->format->palette = NULL;
    s->format->Rmask = Rmask; s->format->Rshift = maskToShift(Rmask); s->format->Rloss = 0;
    s->format->Gmask = Gmask; s->format->Gshift = maskToShift(Gmask); s->format->Gloss = 0;
    s->format->Bmask = Bmask; s->format->Bshift = maskToShift(Bmask); s->format->Bloss = 0;
    s->format->Amask = Amask; s->format->Ashift = maskToShift(Amask); s->format->Aloss = 0;
  }

  s->format->BitsPerPixel = depth;
  s->format->BytesPerPixel = depth / 8;

  s->w = width;
  s->h = height;
  s->pitch = width * depth / 8;
  assert(s->pitch == width * s->format->BytesPerPixel);

  if (!(flags & SDL_PREALLOC)) {
    s->pixels = malloc(s->pitch * height);
    assert(s->pixels);
  }

  return s;
}

SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
    int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  SDL_Surface *s = SDL_CreateRGBSurface(SDL_PREALLOC, width, height, depth,
      Rmask, Gmask, Bmask, Amask);
  assert(pitch == s->pitch);
  s->pixels = pixels;
  return s;
}

void SDL_FreeSurface(SDL_Surface *s) {
  if (s != NULL) {
    if (s->format != NULL) {
      if (s->format->palette != NULL) {
        if (s->format->palette->colors != NULL) free(s->format->palette->colors);
        free(s->format->palette);
      }
      free(s->format);
    }
    if (s->pixels != NULL && !(s->flags & SDL_PREALLOC)) free(s->pixels);
    free(s);
  }
}

SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags) {
  if (flags & SDL_HWSURFACE) NDL_OpenCanvas(&width, &height);
  return SDL_CreateRGBSurface(flags, width, height, bpp,
      DEFAULT_RMASK, DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
}

void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(src && dst);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  assert(dst->format->BitsPerPixel == 8);

  int x = (srcrect == NULL ? 0 : srcrect->x);
  int y = (srcrect == NULL ? 0 : srcrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);

  assert(dstrect);
  assert(x >= 0 && y >= 0 && w >= 0 && h >= 0);
  assert(x + w <= src->w && y + h <= src->h);
  assert(dstrect->x >= 0 && dstrect->y >= 0 && dstrect->w >= 0 && dstrect->h >= 0);
  assert(dstrect->x + dstrect->w <= dst->w && dstrect->y + dstrect->h <= dst->h);
  if(w == dstrect->w && h == dstrect->h) {
    /* The source rectangle and the destination rectangle
     * are of the same size. If that is the case, there
     * is no need to stretch, just copy. */
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_BlitSurface(src, &rect, dst, dstrect);
  } else {
    for (int dst_y = 0; dst_y < dstrect->h; dst_y++) {
      int src_y = y + dst_y * h / dstrect->h;
      uint8_t *dst_row = dst->pixels + (dstrect->y + dst_y) * dst->pitch + dstrect->x;
      uint8_t *src_row = src->pixels + src_y * src->pitch + x;
      for (int dst_x = 0; dst_x < dstrect->w; dst_x++) {
        dst_row[dst_x] = src_row[dst_x * w / dstrect->w];
      }
    }
  }
}

void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
  assert(s);
  assert(s->format);
  assert(s->format->palette);
  assert(colors != NULL);
  assert(firstcolor >= 0 && ncolors >= 0);
  assert(firstcolor + ncolors <= 256);

  memcpy(s->format->palette->colors + firstcolor, colors, sizeof(SDL_Color) * ncolors);

  if(s->flags & SDL_HWSURFACE) {
    SDL_UpdateRect(s, 0, 0, 0, 0);
  }
}

static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len) {
  int i;
  uint8_t (*pdst)[4] = dst;
  uint8_t (*psrc)[4] = src;
  union {
    uint8_t val8[4];
    uint32_t val32;
  } tmp;
  int first = len & ~0xf;
  for (i = 0; i < first; i += 16) {
#define macro(i) \
    tmp.val32 = *((uint32_t *)psrc[i]); \
    *((uint32_t *)pdst[i]) = tmp.val32; \
    pdst[i][0] = tmp.val8[2]; \
    pdst[i][2] = tmp.val8[0];

    macro(i + 0); macro(i + 1); macro(i + 2); macro(i + 3);
    macro(i + 4); macro(i + 5); macro(i + 6); macro(i + 7);
    macro(i + 8); macro(i + 9); macro(i +10); macro(i +11);
    macro(i +12); macro(i +13); macro(i +14); macro(i +15);
  }

  for (; i < len; i ++) {
    macro(i);
  }
}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags) {
  assert(src->format->BitsPerPixel == 32);
  assert(src->w * src->format->BytesPerPixel == src->pitch);
  assert(src->format->BitsPerPixel == fmt->BitsPerPixel);

  SDL_Surface* ret = SDL_CreateRGBSurface(flags, src->w, src->h, fmt->BitsPerPixel,
    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

  assert(fmt->Gmask == src->format->Gmask);
  assert(fmt->Amask == 0 || src->format->Amask == 0 || (fmt->Amask == src->format->Amask));
  ConvertPixelsARGB_ABGR(ret->pixels, src->pixels, src->w * src->h);

  return ret;
}

uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  assert(fmt->BytesPerPixel == 4);
  uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
  if (fmt->Amask) p |= (a << fmt->Ashift);
  return p;
}

int SDL_LockSurface(SDL_Surface *s) {
  return 0;
}

void SDL_UnlockSurface(SDL_Surface *s) {
}
