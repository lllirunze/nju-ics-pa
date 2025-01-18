#include <common.h>
#include <device.h>

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
  char *data = (char *)buf;
  size_t i;
  for (i=0; i<len; i++) {
    putch(*data);
    data++;
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) return 0;
  int _len = snprintf(buf, len, "%s %s", (ev.keydown ? "kd" : "ku"), keyname[ev.keycode]);
  return _len;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  /**
   * todo: get the size of screen
   * 1. write `buf` with `len` size.
   * 2. ignore `offset`
   * 3. We assume that the file doesn't support `lseek`
   */
  AM_GPU_CONFIG_T gc = io_read(AM_GPU_CONFIG);
  if (gc.present == false) panic("should not reach here");
  int width = gc.width, height = gc.height;
  int _len = snprintf(buf, len, "WIDTH: %d\nHEIGHT: %d\n", width, height);
  return _len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

intptr_t timer_read(struct timeval *tv, struct timezone *tz) {
  if (!tv) return -1;
  uint64_t sec = io_read(AM_TIMER_UPTIME).us;
  tv->tv_sec = sec / 1000000;
  tv->tv_usec = sec % 1000000;
  // todo: here we just set the default timezone.
  if (tz) {
    tz->tz_dsttime = 0;
    tz->tz_minuteswest = 0;
  }
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
