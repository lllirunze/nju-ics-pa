#include <am.h>
#include <klib.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t size = inl(VGACTL_ADDR);
  int width = size >> 16;
  int height = size & 0xffff;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = width * height * (int)sizeof(uint32_t)
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  AM_GPU_CONFIG_T cfg;
  __am_gpu_config(&cfg);

  if (ctl->pixels != NULL && ctl->x >= 0 && ctl->y >= 0 && ctl->w > 0 && ctl->h > 0 &&
      ctl->x < cfg.width && ctl->y < cfg.height) {
    int width = ctl->w;
    int height = ctl->h;
    if (ctl->x + width > cfg.width) width = cfg.width - ctl->x;
    if (ctl->y + height > cfg.height) height = cfg.height - ctl->y;

    uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
    uint32_t *pixels = ctl->pixels;
    for (int y = 0; y < height; y ++) {
      memcpy(fb + (ctl->y + y) * cfg.width + ctl->x,
          pixels + y * ctl->w, width * sizeof(uint32_t));
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
