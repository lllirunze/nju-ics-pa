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
  /**
   * Serial port is a character device, 
   * the corresponding byte sequence doesn't have the concept of "position",
   * so the offset parameter ca be ignored.
   */
  char *data = (char *)buf;
  size_t i;
  for (i=0; i<len; i++) {
    putch(*data);
    data++;
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
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
