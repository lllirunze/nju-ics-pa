#include <SDL_mixer.h>
#include <vorbis.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  Mix_Chunk *chunk;
  uint64_t position;
  int loops;
  int volume;
  int playing;
} Channel;

typedef struct {
  Mix_Music *music;
  uint64_t position;
  int loops;
  int volume;
  int playing;
} MusicPlayer;

static SDL_AudioSpec audio_spec;
static Channel *channels = NULL;
static int channel_count = 0;
static MusicPlayer music_player;
static void (*channel_finished)(int channel) = NULL;
static void (*music_finished)(void) = NULL;
static char error_message[128];

static void set_error(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vsnprintf(error_message, sizeof(error_message), format, ap);
  va_end(ap);
}

static uint16_t read_le16(const uint8_t *p) {
  return p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t read_le32(const uint8_t *p) {
  return p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
      ((uint32_t)p[3] << 24);
}

static uint8_t *read_rwops(SDL_RWops *src, int *length) {
  if (src == NULL || length == NULL) return NULL;
  int64_t size = SDL_RWsize(src);
  if (size <= 0 || size > 0x7fffffff || SDL_RWseek(src, 0, RW_SEEK_SET) < 0) return NULL;
  uint8_t *data = malloc((size_t)size);
  if (data == NULL) return NULL;
  if (src->read(src, data, 1, (size_t)size) != (size_t)size) {
    free(data);
    return NULL;
  }
  *length = (int)size;
  return data;
}

static int decode_wav(const uint8_t *data, int length, int *frequency,
    int *channels_out, int *frames, int16_t **samples) {
  if (length < 12 || memcmp(data, "RIFF", 4) != 0 || memcmp(data + 8, "WAVE", 4) != 0) return -1;
  const uint8_t *fmt = NULL, *pcm = NULL;
  uint32_t fmt_size = 0, pcm_size = 0;
  int offset = 12;
  while (offset + 8 <= length) {
    uint32_t size = read_le32(data + offset + 4);
    if (size > (uint32_t)(length - offset - 8)) return -1;
    if (memcmp(data + offset, "fmt ", 4) == 0) { fmt = data + offset + 8; fmt_size = size; }
    else if (memcmp(data + offset, "data", 4) == 0) { pcm = data + offset + 8; pcm_size = size; }
    offset += 8 + size + (size & 1);
  }
  if (fmt == NULL || pcm == NULL || fmt_size < 16 || read_le16(fmt) != 1 ||
      read_le16(fmt + 14) != 16) return -1;
  int ch = read_le16(fmt + 2);
  int rate = read_le32(fmt + 4);
  if (ch <= 0 || rate <= 0 || pcm_size / sizeof(int16_t) < (uint32_t)ch) return -1;
  int16_t *output = malloc(pcm_size);
  if (output == NULL) return -1;
  memcpy(output, pcm, pcm_size);
  *frequency = rate;
  *channels_out = ch;
  *frames = pcm_size / (sizeof(int16_t) * ch);
  *samples = output;
  return 0;
}

static int decode_audio(const uint8_t *data, int length, int *frequency,
    int *channels_out, int *frames, int16_t **samples) {
  if (length >= 4 && memcmp(data, "OggS", 4) == 0) {
    int rate, ch;
    short *output = NULL;
    int count = stb_vorbis_decode_memory(data, length, &ch, &rate, &output);
    if (count <= 0 || output == NULL || ch <= 0 || rate <= 0) return -1;
    *frequency = rate;
    *channels_out = ch;
    *frames = count;
    *samples = output;
    return 0;
  }
  return decode_wav(data, length, frequency, channels_out, frames, samples);
}

static Mix_Chunk *load_chunk(SDL_RWops *src, int freesrc) {
  int length = 0;
  uint8_t *data = read_rwops(src, &length);
  if (freesrc && src != NULL) src->close(src);
  if (data == NULL) { set_error("cannot read audio data"); return NULL; }
  Mix_Chunk *chunk = calloc(1, sizeof(*chunk));
  if (chunk == NULL || decode_audio(data, length, &chunk->frequency, &chunk->channels,
          &chunk->frames, &chunk->samples) != 0) {
    free(data); free(chunk); set_error("unsupported or invalid audio file"); return NULL;
  }
  free(data);
  return chunk;
}

static int sample_at(const int16_t *samples, int frames, int source_channels, int frame, int channel) {
  if (frame < 0 || frame >= frames) return 0;
  if (source_channels == 1) return samples[frame];
  if (channel >= source_channels) channel = source_channels - 1;
  return samples[frame * source_channels + channel];
}

static void mix_track(int16_t *stream, int output_frames, const int16_t *samples,
    int frames, int source_channels, int source_frequency, uint64_t *position,
    int *loops, int *playing, int volume, int is_music, int channel) {
  if (!*playing || samples == NULL || frames <= 0 || source_channels <= 0) return;
  uint64_t step = ((uint64_t)source_frequency << 16) / audio_spec.freq;
  if (step == 0) step = 1;
  for (int out_frame = 0; out_frame < output_frames; out_frame++) {
    int frame = *position >> 16;
    while (frame >= frames) {
      if (*loops == 0) {
        *playing = 0;
        if (is_music) { if (music_finished != NULL) music_finished(); }
        else if (channel_finished != NULL) channel_finished(channel);
        return;
      }
      if (*loops > 0) (*loops)--;
      *position -= (uint64_t)frames << 16;
      frame = *position >> 16;
    }
    int fraction = *position & 0xffff;
    for (int out_channel = 0; out_channel < audio_spec.channels; out_channel++) {
      int left = sample_at(samples, frames, source_channels, frame, out_channel);
      int right = sample_at(samples, frames, source_channels, frame + 1, out_channel);
      int value = left + ((right - left) * fraction >> 16);
      int index = out_frame * audio_spec.channels + out_channel;
      value = stream[index] + value * volume / MIX_MAX_VOLUME;
      if (value > 32767) value = 32767;
      if (value < -32768) value = -32768;
      stream[index] = value;
    }
    *position += step;
  }
}

static void mix_audio(void *userdata, uint8_t *stream, int length) {
  (void)userdata;
  memset(stream, 0, length);
  int frames = length / (sizeof(int16_t) * audio_spec.channels);
  int16_t *output = (int16_t *)stream;
  mix_track(output, frames, music_player.music ? music_player.music->samples : NULL,
      music_player.music ? music_player.music->frames : 0, music_player.music ? music_player.music->channels : 0,
      music_player.music ? music_player.music->frequency : 0, &music_player.position, &music_player.loops,
      &music_player.playing, music_player.volume, 1, -1);
  for (int i = 0; i < channel_count; i++) {
    Channel *ch = &channels[i];
    mix_track(output, frames, ch->chunk ? ch->chunk->samples : NULL, ch->chunk ? ch->chunk->frames : 0,
        ch->chunk ? ch->chunk->channels : 0, ch->chunk ? ch->chunk->frequency : 0, &ch->position,
        &ch->loops, &ch->playing, ch->volume, 0, i);
  }
}

int Mix_OpenAudio(int frequency, uint16_t format, int channels_out, int chunksize) {
  if (frequency <= 0 || format != MIX_DEFAULT_FORMAT || channels_out <= 0 || chunksize <= 0) {
    set_error("unsupported audio format"); return -1;
  }
  Mix_CloseAudio();
  memset(&audio_spec, 0, sizeof(audio_spec));
  audio_spec.freq = frequency; audio_spec.format = format; audio_spec.channels = channels_out;
  audio_spec.samples = chunksize; audio_spec.callback = mix_audio;
  if (SDL_OpenAudio(&audio_spec, NULL) != 0) { set_error("SDL_OpenAudio failed"); return -1; }
  SDL_PauseAudio(0);
  error_message[0] = '\0';
  return 0;
}

void Mix_CloseAudio() {
  SDL_CloseAudio();
  free(channels); channels = NULL; channel_count = 0;
  memset(&music_player, 0, sizeof(music_player));
  memset(&audio_spec, 0, sizeof(audio_spec));
}

char *Mix_GetError() { return error_message; }

int Mix_QuerySpec(int *frequency, uint16_t *format, int *channels_out) {
  if (audio_spec.freq == 0) return 0;
  if (frequency != NULL) *frequency = audio_spec.freq;
  if (format != NULL) *format = audio_spec.format;
  if (channels_out != NULL) *channels_out = audio_spec.channels;
  return 1;
}

Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc) { return load_chunk(src, freesrc); }

void Mix_FreeChunk(Mix_Chunk *chunk) {
  if (chunk == NULL) return;
  for (int i = 0; i < channel_count; i++) if (channels[i].chunk == chunk) channels[i].playing = 0;
  free(chunk->samples); free(chunk);
}

int Mix_AllocateChannels(int numchans) {
  if (numchans < 0) return channel_count;
  Channel *new_channels = calloc((size_t)numchans, sizeof(*new_channels));
  if (numchans != 0 && new_channels == NULL) return channel_count;
  int copied = numchans < channel_count ? numchans : channel_count;
  for (int i = 0; i < copied; i++) new_channels[i] = channels[i];
  for (int i = copied; i < numchans; i++) new_channels[i].volume = MIX_MAX_VOLUME;
  free(channels); channels = new_channels; channel_count = numchans;
  return channel_count;
}

int Mix_Volume(int channel, int volume) {
  if (channel < 0 || channel >= channel_count) return -1;
  int previous = channels[channel].volume;
  if (volume >= 0) { if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME; channels[channel].volume = volume; }
  return previous;
}

int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops) {
  if (chunk == NULL) return -1;
  if (channel == -1) for (int i = 0; i < channel_count; i++) if (!channels[i].playing) { channel = i; break; }
  if (channel < 0 || channel >= channel_count) return -1;
  channels[channel].chunk = chunk; channels[channel].position = 0;
  channels[channel].loops = loops; channels[channel].playing = 1;
  return channel;
}

void Mix_Pause(int channel) {
  if (channel == -1) for (int i = 0; i < channel_count; i++) channels[i].playing = 0;
  else if (channel >= 0 && channel < channel_count) channels[channel].playing = 0;
}

void Mix_ChannelFinished(void (*callback)(int channel)) { channel_finished = callback; }

Mix_Music *Mix_LoadMUS(const char *file) {
  SDL_RWops *src = SDL_RWFromFile(file, "rb");
  if (src == NULL) { set_error("cannot open music file"); return NULL; }
  return Mix_LoadMUS_RW(src);
}

Mix_Music *Mix_LoadMUS_RW(SDL_RWops *src) { return (Mix_Music *)load_chunk(src, 1); }

void Mix_FreeMusic(Mix_Music *music) {
  if (music == NULL) return;
  if (music_player.music == music) music_player.playing = 0;
  free(music->samples); free(music);
}

int Mix_PlayMusic(Mix_Music *music, int loops) {
  if (music == NULL || audio_spec.freq == 0) return -1;
  music_player.music = music; music_player.position = 0; music_player.loops = loops; music_player.playing = 1;
  if (music_player.volume == 0) music_player.volume = MIX_MAX_VOLUME;
  return 0;
}

int Mix_SetMusicPosition(double position) {
  if (music_player.music == NULL || position < 0) return -1;
  uint64_t frame = (uint64_t)(position * music_player.music->frequency);
  if (frame >= (uint64_t)music_player.music->frames) return -1;
  music_player.position = frame << 16;
  return 0;
}

int Mix_VolumeMusic(int volume) {
  int previous = music_player.volume;
  if (volume >= 0) { if (volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME; music_player.volume = volume; }
  return previous;
}

int Mix_SetMusicCMD(const char *command) { (void)command; return 0; }
int Mix_HaltMusic() { music_player.playing = 0; return 0; }
void Mix_HookMusicFinished(void (*callback)()) { music_finished = callback; }
int Mix_PlayingMusic() { return music_player.playing; }
