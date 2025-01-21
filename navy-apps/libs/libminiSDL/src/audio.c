#include <NDL.h>
#include <SDL.h>

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  // panic("not implemented\n");
  return 0;
}

void SDL_CloseAudio() {
  // panic("not implemented\n");
}

void SDL_PauseAudio(int pause_on) {
  // panic("not implemented\n");
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
  // panic("not implemented\n");
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
  // panic("not implemented\n");
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
  // panic("not implemented\n");
}

void SDL_LockAudio() {
  // panic("not implemented\n");
}

void SDL_UnlockAudio() {
  // panic("not implemented\n");
}
