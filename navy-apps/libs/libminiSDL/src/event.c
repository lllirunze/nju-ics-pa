#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

#define NR_KEYS (sizeof(keyname) / sizeof(keyname[0]))
#define EVENT_QUEUE_SIZE 64

static SDL_Event event_queue[EVENT_QUEUE_SIZE];
static int event_head = 0;
static int event_tail = 0;
static uint8_t key_state[NR_KEYS] = {};

static bool queue_empty(void) {
  return event_head == event_tail;
}

static bool queue_full(void) {
  return (event_tail + 1) % EVENT_QUEUE_SIZE == event_head;
}

static int queue_push(const SDL_Event *ev) {
  if (queue_full()) return -1;
  event_queue[event_tail] = *ev;
  event_tail = (event_tail + 1) % EVENT_QUEUE_SIZE;
  return 0;
}

static int queue_pop(SDL_Event *ev) {
  if (queue_empty()) return 0;
  if (ev != NULL) *ev = event_queue[event_head];
  event_head = (event_head + 1) % EVENT_QUEUE_SIZE;
  return 1;
}

static int keycode_from_name(const char *name) {
  for (int i = 0; i < NR_KEYS; i++) {
    if (strcmp(name, keyname[i]) == 0) return i;
  }
  return SDLK_NONE;
}

static int poll_ndl_event(SDL_Event *ev) {
  char buf[64] = {};
  if (!NDL_PollEvent(buf, sizeof(buf) - 1)) return 0;

  bool key_down;
  if (strncmp(buf, "kd ", 3) == 0) {
    key_down = true;
  } else if (strncmp(buf, "ku ", 3) == 0) {
    key_down = false;
  } else {
    return 0;
  }

  char *name = buf + 3;
  name[strcspn(name, "\r\n")] = '\0';
  int keycode = keycode_from_name(name);
  if (keycode == SDLK_NONE) return 0;

  key_state[keycode] = key_down;
  if (ev != NULL) {
    ev->key.type = key_down ? SDL_KEYDOWN : SDL_KEYUP;
    ev->key.keysym.sym = keycode;
  }
  return 1;
}

int SDL_PushEvent(SDL_Event *ev) {
  if (ev == NULL || queue_push(ev) != 0) return -1;
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  if (queue_pop(ev)) return 1;
  return poll_ndl_event(ev);
}

int SDL_WaitEvent(SDL_Event *event) {
  while (!SDL_PollEvent(event)) {
  }
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  if (numevents <= 0 || ev == NULL) return 0;

  if (action == SDL_ADDEVENT) {
    int added = 0;
    while (added < numevents && queue_push(&ev[added]) == 0) added++;
    return added;
  }

  int copied = 0;
  int index = event_head;
  while (index != event_tail && copied < numevents) {
    SDL_Event *queued = &event_queue[index];
    if (mask & SDL_EVENTMASK(queued->type)) ev[copied++] = *queued;
    index = (index + 1) % EVENT_QUEUE_SIZE;
  }
  if (action == SDL_GETEVENT) {
    int removed = 0;
    int queued_events = (event_tail - event_head + EVENT_QUEUE_SIZE) % EVENT_QUEUE_SIZE;
    for (int i = 0; i < queued_events; i++) {
      SDL_Event queued;
      queue_pop(&queued);
      if ((mask & SDL_EVENTMASK(queued.type)) && removed < copied) {
        removed++;
      } else {
        queue_push(&queued);
      }
    }
  }
  return copied;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if (numkeys != NULL) *numkeys = NR_KEYS;
  return key_state;
}
