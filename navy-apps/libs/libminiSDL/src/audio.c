#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SDL_AudioSpec audio_spec;
static uint8_t *audio_stream = NULL;
static uint32_t callback_interval = 0;
static uint32_t last_callback = 0;
static int audio_open = 0;
static int audio_paused = 1;
static int audio_locked = 0;
static int in_callback = 0;

static uint16_t read_le16(const uint8_t *p) {
  return p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t read_le32(const uint8_t *p) {
  return p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
      ((uint32_t)p[3] << 24);
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  if (desired == NULL || desired->callback == NULL || desired->freq <= 0 ||
      desired->channels == 0 || desired->samples == 0 || desired->format != AUDIO_S16SYS) {
    return -1;
  }

  SDL_CloseAudio();
  audio_spec = *desired;
  audio_spec.size = audio_spec.samples * audio_spec.channels * sizeof(int16_t);
  audio_stream = malloc(audio_spec.size);
  if (audio_stream == NULL) return -1;

  NDL_OpenAudio(audio_spec.freq, audio_spec.channels, audio_spec.samples);
  callback_interval = 1000 * audio_spec.samples / audio_spec.freq;
  if (callback_interval == 0) callback_interval = 1;
  last_callback = NDL_GetTicks();
  audio_open = 1;
  audio_paused = 1;
  if (obtained != NULL) *obtained = audio_spec;
  return 0;
}

void SDL_CloseAudio() {
  if (!audio_open) return;
  NDL_CloseAudio();
  free(audio_stream);
  audio_stream = NULL;
  audio_open = 0;
  audio_paused = 1;
  audio_locked = 0;
}

void SDL_PauseAudio(int pause_on) {
  audio_paused = pause_on != 0;
  last_callback = NDL_GetTicks();
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
  assert(dst != NULL && src != NULL);
  if (volume < 0) volume = 0;
  if (volume > SDL_MIX_MAXVOLUME) volume = SDL_MIX_MAXVOLUME;
  uint32_t samples = len / sizeof(int16_t);
  int16_t *out = (int16_t *)dst;
  const int16_t *in = (const int16_t *)src;
  for (uint32_t i = 0; i < samples; i++) {
    int value = out[i] + in[i] * volume / SDL_MIX_MAXVOLUME;
    if (value > 32767) value = 32767;
    if (value < -32768) value = -32768;
    out[i] = value;
  }
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
  if (file == NULL || spec == NULL || audio_buf == NULL || audio_len == NULL) return NULL;
  FILE *fp = fopen(file, "rb");
  if (fp == NULL) return NULL;

  uint8_t header[12];
  if (fread(header, 1, sizeof(header), fp) != sizeof(header) ||
      memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVE", 4) != 0) {
    fclose(fp);
    return NULL;
  }

  uint8_t fmt[16];
  int got_fmt = 0;
  while (!got_fmt) {
    uint8_t chunk[8];
    if (fread(chunk, 1, sizeof(chunk), fp) != sizeof(chunk)) break;
    uint32_t size = read_le32(chunk + 4);
    if (memcmp(chunk, "fmt ", 4) == 0 && size >= sizeof(fmt) &&
        fread(fmt, 1, sizeof(fmt), fp) == sizeof(fmt)) {
      if (size > sizeof(fmt)) fseek(fp, size - sizeof(fmt), SEEK_CUR);
      got_fmt = 1;
    } else {
      fseek(fp, size, SEEK_CUR);
    }
    if (size & 1) fseek(fp, 1, SEEK_CUR);
  }

  if (!got_fmt || read_le16(fmt) != 1 || read_le16(fmt + 14) != 16) {
    fclose(fp);
    return NULL;
  }

  uint8_t *data = NULL;
  uint32_t data_size = 0;
  while (data == NULL) {
    uint8_t chunk[8];
    if (fread(chunk, 1, sizeof(chunk), fp) != sizeof(chunk)) break;
    uint32_t size = read_le32(chunk + 4);
    if (memcmp(chunk, "data", 4) == 0) {
      data = malloc(size);
      if (data != NULL && fread(data, 1, size, fp) != size) {
        free(data);
        data = NULL;
      }
      data_size = size;
    } else {
      fseek(fp, size, SEEK_CUR);
    }
    if (size & 1) fseek(fp, 1, SEEK_CUR);
  }
  fclose(fp);
  if (data == NULL) return NULL;

  spec->freq = read_le32(fmt + 4);
  spec->channels = read_le16(fmt + 2);
  spec->format = AUDIO_S16SYS;
  spec->samples = 0;
  spec->size = data_size;
  spec->callback = NULL;
  spec->userdata = NULL;
  *audio_buf = data;
  *audio_len = data_size;
  return spec;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
  free(audio_buf);
}

void SDL_LockAudio() {
  audio_locked = 1;
}

void SDL_UnlockAudio() {
  audio_locked = 0;
}

void SDL_AudioCallbackHelper() {
  if (!audio_open || audio_paused || audio_locked || in_callback) return;
  uint32_t now = NDL_GetTicks();
  if (now - last_callback < callback_interval) return;

  in_callback = 1;
  memset(audio_stream, 0, audio_spec.size);
  audio_spec.callback(audio_spec.userdata, audio_stream, audio_spec.size);
  NDL_PlayAudio(audio_stream, audio_spec.size);
  last_callback = now;
  in_callback = 0;
}
