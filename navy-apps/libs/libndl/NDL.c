#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

static struct timeval tv;
static struct timezone tz;
static uint64_t start_usec, current_usec;

static int fd_events = -1;

uint32_t NDL_GetTicks() {
  gettimeofday(&tv, &tz);
  current_usec = tv.tv_sec * 1000000 + tv.tv_usec;
  // return microseconds
  return (current_usec-start_usec)/1000;
}

int NDL_PollEvent(char *buf, int len) {
  /**
   * todo: abstract the input into files
   * this api read an event information from `/dev/events`.
   * 1. write events to `buf` with `len` bytes
   * 2. If reading a valid event, return 1.
   *    Otherwise return 0.
   */
  assert(fd_events >= 0);
  if (read(fd_events, buf, len) > 0) return 1;
  return 0;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  gettimeofday(&tv, &tz);
  start_usec = tv.tv_sec * 1000000 + tv.tv_usec;
  fd_events = open("/dev/events", O_RDONLY);
  return 0;
}

void NDL_Quit() {
  close(fd_events);
}
