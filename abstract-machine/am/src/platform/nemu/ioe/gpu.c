#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {

  uint32_t gpu_config = inl(VGACTL_ADDR);
  int w = (gpu_config >> 16) & 0xffff;
  int h = gpu_config & 0xffff;

  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {

  int x = ctl->x, y = ctl->y;
  int w = ctl->w, h = ctl->h;
  gpuptr_t *pixels = ctl->pixels;
  bool sync = ctl->sync;

  if (sync == false && (w == 0 || h == 0)) return;

  int screen_width = (inl(VGACTL_ADDR) >> 16) & 0xffff;

  int i, j;
  for (i=0; i<h; i++) {
    for (j=0; j<w; j++) {
      int screen_pos = (y+i) * screen_width + (x+j);
      uintptr_t addr = FB_ADDR + screen_pos * sizeof(uint32_t);
      outl(addr, pixels[i*w + j]);
    }
  }

  if (sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
