#include <NDL.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

#define keysize (sizeof(keyname) / sizeof(keyname[0]))

int SDL_PushEvent(SDL_Event *ev) {
  // panic("not implemented\n");

  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  // panic("not implemented\n");
  char buf[64];
  if (!NDL_PollEvent(buf, sizeof(buf))) return 0;
  int len = strlen(buf);
  buf[len-1] = '\0';
  ev->type = (buf[1] == 'd') ? SDL_KEYDOWN : SDL_KEYUP;
  int i;
  for (i=0; i<keysize; i++) {
    if (strcmp(keyname[i], buf+3) == 0) {
      ev->key.keysym.sym = i;
      break;
    }
  }
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  // panic("not implemented\n");
  char buf[64];
  while (!NDL_PollEvent(buf, sizeof(buf)));
  int len = strlen(buf);
  buf[len-1] = '\0';
  event->type = (buf[1] == 'd') ? SDL_KEYDOWN : SDL_KEYUP;
  int i;
  for (i=0; i<keysize; i++) {
    if (strcmp(keyname[i], buf+3) == 0) {
      event->key.keysym.sym = i;
      break;
    }
  }
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  // panic("not implemented\n");

  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  // panic("not implemented\n");

  return NULL;
}
