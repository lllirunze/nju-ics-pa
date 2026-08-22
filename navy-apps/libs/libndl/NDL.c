#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static int evtdev = -1;
static int fbdev = -1;
static int sbdev = -1;
static int sbctldev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_x = 0, canvas_y = 0;

#define EVENT_BUFFER_SIZE 256
static char event_buffer[EVENT_BUFFER_SIZE];
static int event_buffer_len = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  assert(buf != NULL && len > 1);

  // The event device can return several newline-terminated events in one
  // read.  Keep the unread suffix so each call still returns exactly one.
  while (1) {
    for (int i = 0; i < event_buffer_len; i++) {
      if (event_buffer[i] != '\n') continue;
      int event_len = i + 1;
      assert(event_len < len);
      memcpy(buf, event_buffer, event_len);
      memmove(event_buffer, event_buffer + event_len, event_buffer_len - event_len);
      event_buffer_len -= event_len;
      return 1;
    }

    if (event_buffer_len == EVENT_BUFFER_SIZE) {
      // Every NDL event is short and newline-terminated.  A full buffer
      // without a newline therefore indicates malformed device input.
      assert(0);
    }
    int nread = read(evtdev, event_buffer + event_buffer_len,
        EVENT_BUFFER_SIZE - event_buffer_len);
    if (nread <= 0) return 0;
    event_buffer_len += nread;
  }
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
    return;
  }

  int dispinfo = open("/proc/dispinfo", 0, 0);
  char buf[64];
  int nread = read(dispinfo, buf, sizeof(buf) - 1);
  assert(nread > 0);
  buf[nread] = '\0';
  close(dispinfo);
  assert(sscanf(buf, "WIDTH:%d\nHEIGHT:%d", &screen_w, &screen_h) == 2);

  if (*w == 0 || *h == 0) {
    *w = screen_w;
    *h = screen_h;
  }
  assert(*w <= screen_w && *h <= screen_h);
  canvas_x = (screen_w - *w) / 2;
  canvas_y = (screen_h - *h) / 2;
  fbdev = open("/dev/fb", 0, 0);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  assert(x >= 0 && y >= 0 && w >= 0 && h >= 0);
  assert(x + w <= screen_w - 2 * canvas_x);
  assert(y + h <= screen_h - 2 * canvas_y);

  for (int row = 0; row < h; row++) {
    off_t offset = ((canvas_y + y + row) * screen_w + canvas_x + x) * sizeof(uint32_t);
    assert(lseek(fbdev, offset, SEEK_SET) == offset);
    assert(write(fbdev, pixels + row * w, w * sizeof(uint32_t)) == w * sizeof(uint32_t));
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
  int params[] = {freq, channels, samples};
  sbctldev = open("/dev/sbctl", 0, 0);
  sbdev = open("/dev/sb", 0, 0);
  assert(sbctldev >= 0 && sbdev >= 0);
  assert(write(sbctldev, params, sizeof(params)) == sizeof(params));
}

void NDL_CloseAudio() {
  if (sbdev >= 0) close(sbdev);
  if (sbctldev >= 0) close(sbctldev);
  sbdev = -1;
  sbctldev = -1;
}

int NDL_PlayAudio(void *buf, int len) {
  assert(sbdev >= 0);
  return write(sbdev, buf, len);
}

int NDL_QueryAudio() {
  int free_bytes = 0;
  assert(sbctldev >= 0);
  assert(read(sbctldev, &free_bytes, sizeof(free_bytes)) == sizeof(free_bytes));
  return free_bytes;
}

int NDL_Init(uint32_t flags) {
  (void)flags;
  if (getenv("NWM_APP")) {
    evtdev = 3;
  } else {
    evtdev = open("/dev/events", 0, 0);
  }
  assert(evtdev >= 0);
  event_buffer_len = 0;
  return 0;
}

void NDL_Quit() {
}
