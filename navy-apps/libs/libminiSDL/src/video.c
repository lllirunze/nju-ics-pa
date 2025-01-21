#include <NDL.h>
#include <sdl-video.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  
  printf("SDL_BlitSurface\n");

  assert(dst && src);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);

  uint32_t w, h;
  if (!srcrect) { w = src->w; h = src->h; }
  else { w = srcrect->w; h = srcrect->h; }
  assert(w <= dst->w && h <= dst->h);
  
  int32_t x, y;
  if (!dstrect) { x = 0; y = 0; }
  else { x = dstrect->x; y = dstrect->y; }
  
  uint32_t i;
  for (i=0; i<h; i++) {
    memcpy(dst->pixels + x*sizeof(uint32_t) + (y+i)*(dst->w)*sizeof(uint32_t), src->pixels + i*w*sizeof(uint32_t), w*sizeof(uint32_t));
  }

}

void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color) {

  printf("SDL_FillRect\n");

  uint32_t w, h;
  int32_t x, y;
  if (!dstrect) { x = 0; y = 0; w = dst->w; h = dst->h; }
  else { x = dstrect->x; y = dstrect->y; w = dstrect->w; h = dstrect->h; }
  assert(w <= dst->w && h <= dst->h);
  
  uint32_t i, j;
  for (i=y; i<y+h; i++) {
    for (j=x; j<x+w; j++) {
      *((uint32_t *)dst->pixels + i*w + j) = color;
    }
  }
}

/*
void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h) {
  if (!x && !y && !w && !h) {
    w = s->w;
    h = s->h;
  }
  uint32_t* pixels = (uint32_t*)s->pixels;
  if (s->format->BitsPerPixel == 8) {
    pixels = (uint32_t*)malloc(sizeof(uint32_t) * s->w * s->h);
    SDL_Color *colors = s->format->palette->colors;
    for (int i = 0; i < s->w * s->h; ++ i) {
      pixels[i] = (colors[s->pixels[i]].a << 24) | (colors[s->pixels[i]].r << 16) | (colors[s->pixels[i]].g << 8) | colors[s->pixels[i]].b;
      // if direct assign, the color would be 0x00BBGGRR
    }
  }
  NDL_DrawRect(pixels, x, y, w, h);
  if (s->format->BitsPerPixel == 8)
    free(pixels);
}
*/

void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h) {

  printf("SDL_UpdateRect\n");

  if (x == 0 && y == 0 && w == 0 && h == 0) {w = s->w; h = s->h;}
  NDL_DrawRect((uint32_t *)(s->pixels), x, y, w, h);
  // uint32_t* pixels = (uint32_t*)s->pixels;
  // if (s->format->BitsPerPixel == 8) {
  //   pixels = (uint32_t*)malloc(sizeof(uint32_t) * s->w * s->h);
  //   SDL_Color *colors = s->format->palette->colors;
  //   for (int i = 0; i < s->w * s->h; ++ i) {
  //     pixels[i] = (colors[s->pixels[i]].a << 24) | (colors[s->pixels[i]].r << 16) | (colors[s->pixels[i]].g << 8) | colors[s->pixels[i]].b;
  //     // if direct assign, the color would be 0x00BBGGRR
  //   }
  // }
  // NDL_DrawRect(pixels, x, y, w, h);
  // if (s->format->BitsPerPixel == 8) free(pixels);
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

  printf("SDL_CreateRGBSurface\n");

  assert(depth == 8 || depth == 32);
  SDL_Surface *s = malloc(sizeof(SDL_Surface));
  assert(s);
  s->flags = flags;
  s->format = malloc(sizeof(SDL_PixelFormat));
  assert(s->format);
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
  
  printf("SDL_SoftStretch\n");
  
  assert(src && dst);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  assert(dst->format->BitsPerPixel == 8);

  int x = (srcrect == NULL ? 0 : srcrect->x);
  int y = (srcrect == NULL ? 0 : srcrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);

  assert(dstrect);
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
  }
  else {
    printf("%d %d %d %d\n",w , dstrect->w ,h , dstrect->h);
    // it seems that, must set pal window size to 320 * 200
    // or this unimplemented strech will fail
    assert(0);
  }
}

void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
  
  printf("SDL_SetPalette\n");

  assert(s);
  assert(s->format);
  assert(s->format->palette);
  assert(firstcolor == 0);

  s->format->palette->ncolors = ncolors;
  memcpy(s->format->palette->colors, colors, sizeof(SDL_Color) * ncolors);

  if(s->flags & SDL_HWSURFACE) {
    assert(ncolors == 256);
    for (int i = 0; i < ncolors; i ++) {
      uint8_t r = colors[i].r;
      uint8_t g = colors[i].g;
      uint8_t b = colors[i].b;
    }
    SDL_UpdateRect(s, 0, 0, 0, 0);
  }
}

static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len) {

  printf("ConvertPixelsARGB_ABGR\n");

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
  
  printf("SDL_ConvertSurface\n");
  
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
  
  printf("SDL_MapRGBA\n");
  
  assert(fmt->BytesPerPixel == 4);
  uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
  if (fmt->Amask) p |= (a << fmt->Ashift);
  return p;
}

int SDL_LockSurface(SDL_Surface *s) {
  // panic("not implemented\n");

  return 0;
}

void SDL_UnlockSurface(SDL_Surface *s) {
  // panic("not implemented\n");

}
