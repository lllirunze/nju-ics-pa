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

static uint8_t keystate[keysize];

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  if (!NDL_PollEvent(buf, sizeof(buf))) return 0;
  char type[4], name[32];
  sscanf(buf, "%s %s\n", type, name);
  int i;
  for (i=0; i<keysize; i++) {
    if (strcmp(keyname[i], name) == 0) {
      ev->key.keysym.sym = i;
      break;
    }
  }
  ev->type = (type[1] == 'd') ? SDL_KEYDOWN : SDL_KEYUP;
  ev->key.type = (type[1] == 'd') ? SDL_KEYDOWN : SDL_KEYUP;
  keystate[ev->key.keysym.sym] = (type[1] == 'd') ? 1 : 0;
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[64];
  while (!NDL_PollEvent(buf, sizeof(buf)));
  char type[4], name[32];
  sscanf(buf, "%s %s\n", type, name);
  int i;
  for (i=0; i<keysize; i++) {
    if (strcmp(keyname[i], name) == 0) {
      event->key.keysym.sym = i;
      break;
    }
  }
  event->type = (type[1] == 'd') ? SDL_KEYDOWN : SDL_KEYUP;
  event->key.type = (type[1] == 'd') ? SDL_KEYDOWN : SDL_KEYUP;
  keystate[event->key.keysym.sym] = (type[1] == 'd') ? 1 : 0;
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if (numkeys) *numkeys = keysize;
  return &keystate;
}
