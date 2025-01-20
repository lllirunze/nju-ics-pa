#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev  = -1;

static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;

static struct timeval tv;
static struct timezone tz;
static uint64_t start_usec, current_usec;

static int fd_events = -1;
static int fd_screen = -1;
static int fd_frmbuf = -1;

uint32_t NDL_GetTicks() {
  gettimeofday(&tv, &tz);
  current_usec = tv.tv_sec * 1000000 + tv.tv_usec;
  // return microseconds
  return (current_usec-start_usec)/1000;
}

int NDL_PollEvent(char *buf, int len) {
  if (read(fd_events, buf, len) > 0) return 1;
  return 0;
}

void NDL_OpenCanvas(int *w, int *h) {

  if(*w == 0 || *w > screen_w) *w = screen_w;
  if(*h == 0 || *h > screen_h) *h = screen_h;
  canvas_w = *w;
  canvas_h = *h; 

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
  assert(x >= 0 && y >= 0 && x+w <= screen_w && y+h <= screen_h);
  int i;
  for (i=0; i<h; i++) {
    lseek(fd_frmbuf, (x + (y+i)*screen_w)*sizeof(uint32_t), SEEK_SET);
    assert(write(fd_frmbuf, (void *)(pixels + i*w), w*sizeof(uint32_t)) >= 0);
  }
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

void NDL_GetScreenSize() {
  char buf[128];
  assert(read(fd_screen, buf, sizeof(buf)) >= 0);
  sscanf(buf, "WIDTH: %d\nHEIGHT: %d\n", &screen_w, &screen_h);
  canvas_w = screen_w;
  canvas_h = screen_h;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  gettimeofday(&tv, &tz);
  start_usec = tv.tv_sec * 1000000 + tv.tv_usec;

  fd_events = open("/dev/events", O_RDONLY);
  assert(fd_events >= 0);
  fd_screen = open("/proc/dispinfo", O_RDONLY);
  assert(fd_screen >= 0);
  fd_frmbuf = open("/dev/fb", O_WRONLY);
  assert(fd_frmbuf >= 0);

  NDL_GetScreenSize();
  
  return 0;
}

void NDL_Quit() {
  close(fd_frmbuf);
  close(fd_screen);
  close(fd_events);
}
