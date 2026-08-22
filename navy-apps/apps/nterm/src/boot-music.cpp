#include <nterm.h>
#include <SDL.h>
#include <vorbis.h>

static const char *music_path = "/share/music/little-star.ogg";
static const uint32_t music_duration_ms = 4000;
static const int music_samples = 4096;

static stb_vorbis *decoder = NULL;
static uint8_t *music_data = NULL;
static uint32_t music_started_at = 0;
static bool music_playing = false;
static bool music_finished = false;

static void stop_boot_music() {
  SDL_PauseAudio(1);
  SDL_CloseAudio();
  stb_vorbis_close(decoder);
  free(music_data);
  decoder = NULL;
  music_data = NULL;
  music_playing = false;
}

static void fill_boot_music(void *userdata, uint8_t *stream, int len) {
  stb_vorbis_info info = stb_vorbis_get_info(decoder);
  int samples = stb_vorbis_get_samples_short_interleaved(
      decoder, info.channels, (int16_t *)stream, len / sizeof(int16_t));
  int nbyte = samples * info.channels * sizeof(int16_t);
  if (samples == 0) music_finished = true;
  if (nbyte < len) memset(stream + nbyte, 0, len - nbyte);
}

void start_boot_music() {
  FILE *fp = fopen(music_path, "r");
  if (!fp) return;

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  if (size <= 0) {
    fclose(fp);
    return;
  }

  music_data = (uint8_t *)malloc(size);
  if (!music_data || fread(music_data, 1, size, fp) != (size_t)size) {
    fclose(fp);
    free(music_data);
    music_data = NULL;
    return;
  }
  fclose(fp);

  int error;
  decoder = stb_vorbis_open_memory(music_data, size, &error, NULL);
  if (!decoder) {
    free(music_data);
    music_data = NULL;
    return;
  }

  stb_vorbis_info info = stb_vorbis_get_info(decoder);
  SDL_AudioSpec spec = {};
  spec.freq = info.sample_rate;
  spec.channels = info.channels;
  spec.samples = music_samples;
  spec.format = AUDIO_S16SYS;
  spec.callback = fill_boot_music;
  if (SDL_OpenAudio(&spec, NULL) < 0) {
    stb_vorbis_close(decoder);
    free(music_data);
    decoder = NULL;
    music_data = NULL;
    return;
  }

  music_started_at = SDL_GetTicks();
  music_finished = false;
  music_playing = true;
  SDL_PauseAudio(0);
}

void poll_boot_music() {
  if (music_playing && (music_finished || SDL_GetTicks() - music_started_at >= music_duration_ms)) {
    stop_boot_music();
  }
}
