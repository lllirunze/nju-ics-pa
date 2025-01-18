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

uint32_t NDL_GetTicks() {
  gettimeofday(&tv, &tz);
  current_usec = tv.tv_sec * 1000000 + tv.tv_usec;
  // return microseconds
  return (current_usec-start_usec)/1000;
}

int NDL_PollEvent(char *buf, int len) {
  fd_events = open("/dev/events", O_RDONLY);
  assert(fd_events >= 0);
  if (read(fd_events, buf, len) > 0) {
    close(fd_events);
    return 1;
  }
  close(fd_events);
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

  /**
   * todo: get the size of screen
   * 1. record the size of canvas
   * 2. this size can't exceed the size of screen.
   * 3. open canvas with <(*w) x (*h)>
   * 4. if both (*w) and (*h) are 0, 
   *    set the screen as canvas 
   *    set (*w) and (*h) as the size of screen
   */
  if (*w == 0 && *h == 0) {
    canvas_w = screen_w;
    canvas_h = screen_h;
    *w = screen_w;
    *h = screen_h;
  }
  else {
    assert((*w) <= screen_w && (*h) <= screen_h);
    canvas_w = *w;
    canvas_h = *h;
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  /**
   * todo: draw the figure
   * 1. draw the figure with `w * h` size at the pixel (x, y)
   * 2. sync this region to screen
   * 3. the pixels are stored in row-based prioritization
   * 4. the pixel's form: `00RRGGBB`
   */
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
  fd_screen = open("/proc/dispinfo", O_RDONLY);
  assert(fd_screen >= 0);
  char buf[128];
  read(fd_screen, buf, sizeof(buf));
  char *str1 = strtok(buf, ":");
  assert(strcmp(str1, "WIDTH") == 0);
  char *str2 = strtok(NULL, "\n");
  assert(str2 != NULL);
  screen_w = atoi(str2);
  char *str3 = strtok(NULL, ":");
  assert(strcmp(str3, "HEIGHT") == 0);
  char *str4 = strtok(NULL, "\n");
  assert(str4 != NULL);
  screen_h = atoi(str4);
  close(fd_screen);
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  gettimeofday(&tv, &tz);
  start_usec = tv.tv_sec * 1000000 + tv.tv_usec;
  NDL_GetScreenSize();
  return 0;
}

void NDL_Quit() {
  
}
