#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  (void)offset;
  const char *str = buf;
  for (size_t i = 0; i < len; i++) {
    putch(str[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  (void)offset;
  if (len == 0) return 0;

  AM_INPUT_KEYBRD_T event = io_read(AM_INPUT_KEYBRD);
  if (event.keycode == AM_KEY_NONE) return 0;

  int n = snprintf(buf, len, "k%c %s\n", event.keydown ? 'd' : 'u',
      keyname[event.keycode]);
  return (size_t)n < len - 1 ? (size_t)n : len - 1;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  (void)offset;
  if (len == 0) return 0;

  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  int n = snprintf(buf, len, "WIDTH:%d\nHEIGHT:%d\n", cfg.width, cfg.height);
  return (size_t)n < len - 1 ? (size_t)n : len - 1;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  assert(offset % sizeof(uint32_t) == 0);
  assert(len % sizeof(uint32_t) == 0);

  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  const uint32_t *pixels = buf;
  size_t pixel_offset = offset / sizeof(uint32_t);
  size_t nr_pixels = len / sizeof(uint32_t);
  assert(pixel_offset + nr_pixels <= (size_t)cfg.width * cfg.height);

  while (nr_pixels > 0) {
    int x = pixel_offset % cfg.width;
    int y = pixel_offset / cfg.width;
    int width = nr_pixels < (size_t)cfg.width - x ? nr_pixels : (size_t)cfg.width - x;
    io_write(AM_GPU_FBDRAW, x, y, (void *)pixels, width, 1, true);
    pixels += width;
    pixel_offset += width;
    nr_pixels -= width;
  }
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
