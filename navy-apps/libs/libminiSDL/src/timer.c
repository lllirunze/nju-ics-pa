#include <NDL.h>
#include <sdl-audio.h>
#include <sdl-timer.h>
#include <stdio.h>

static uint32_t start_ticks = 0;

void SDL_StartTicks() {
  start_ticks = NDL_GetTicks();
}

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  return 1;
}

uint32_t SDL_GetTicks() {
  SDL_AudioCallbackHelper();
  return NDL_GetTicks() - start_ticks;
}

void SDL_Delay(uint32_t ms) {
  uint32_t deadline = SDL_GetTicks() + ms;
  while ((int32_t)(SDL_GetTicks() - deadline) < 0) {
  }
}
