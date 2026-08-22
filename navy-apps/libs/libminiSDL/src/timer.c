#include <NDL.h>
#include <sdl-audio.h>
#include <sdl-timer.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t start_ticks = 0;

typedef struct Timer {
  uint32_t deadline;
  uint32_t interval;
  SDL_NewTimerCallback callback;
  void *param;
  int active;
  struct Timer *next;
} Timer;

static Timer *timers = NULL;
static int dispatching = 0;

void SDL_StartTicks() {
  start_ticks = NDL_GetTicks();
}

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
  if (interval == 0 || callback == NULL) return NULL;
  Timer *timer = malloc(sizeof(*timer));
  if (timer == NULL) return NULL;
  timer->deadline = NDL_GetTicks() - start_ticks + interval;
  timer->interval = interval;
  timer->callback = callback;
  timer->param = param;
  timer->active = 1;
  timer->next = timers;
  timers = timer;
  return timer;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  Timer *timer = id;
  if (timer == NULL) return 0;
  Timer **link = &timers;
  while (*link != NULL && *link != timer) link = &(*link)->next;
  if (*link == NULL) return 0;

  timer->active = 0;
  if (!dispatching) {
    *link = timer->next;
    free(timer);
  }
  return 1;
}

void SDL_TimerCallbackHelper() {
  if (dispatching) return;
  dispatching = 1;
  uint32_t now = NDL_GetTicks() - start_ticks;

  for (Timer *timer = timers; timer != NULL; timer = timer->next) {
    if (!timer->active || (int32_t)(now - timer->deadline) < 0) continue;
    uint32_t interval = timer->callback(timer->interval, timer->param);
    if (!timer->active || interval == 0) {
      timer->active = 0;
    } else {
      timer->interval = interval;
      timer->deadline = now + interval;
    }
  }

  dispatching = 0;
  Timer **link = &timers;
  while (*link != NULL) {
    Timer *timer = *link;
    if (!timer->active) {
      *link = timer->next;
      free(timer);
    } else {
      link = &timer->next;
    }
  }
}

uint32_t SDL_GetTicks() {
  SDL_AudioCallbackHelper();
  SDL_TimerCallbackHelper();
  return NDL_GetTicks() - start_ticks;
}

void SDL_Delay(uint32_t ms) {
  uint32_t deadline = SDL_GetTicks() + ms;
  while ((int32_t)(SDL_GetTicks() - deadline) < 0) {
  }
}
