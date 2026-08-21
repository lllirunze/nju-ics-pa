/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;
static uint32_t sbuf_rpos = 0;
static uint32_t sbuf_count = 0;
static bool audio_open = false;

static void audio_callback(void *userdata, uint8_t *stream, int len) {
  memset(stream, 0, len);

  uint32_t nread = (uint32_t)len < sbuf_count ? (uint32_t)len : sbuf_count;
  uint32_t first = nread < CONFIG_SB_SIZE - sbuf_rpos ? nread : CONFIG_SB_SIZE - sbuf_rpos;
  memcpy(stream, sbuf + sbuf_rpos, first);
  memcpy(stream + first, sbuf, nread - first);

  sbuf_rpos = (sbuf_rpos + nread) % CONFIG_SB_SIZE;
  sbuf_count -= nread;
  audio_base[reg_count] = sbuf_count;
}

static void init_audio_device() {
  SDL_AudioSpec spec = {};
  spec.freq = audio_base[reg_freq];
  spec.format = AUDIO_S16SYS;
  spec.channels = audio_base[reg_channels];
  spec.samples = audio_base[reg_samples];
  spec.callback = audio_callback;
  spec.userdata = NULL;

  Assert(spec.freq > 0 && spec.channels > 0 && spec.samples > 0,
      "invalid audio configuration: freq=%d channels=%d samples=%d",
      spec.freq, spec.channels, spec.samples);

  if (audio_open) {
    SDL_CloseAudio();
    audio_open = false;
  }
  Assert(SDL_InitSubSystem(SDL_INIT_AUDIO) == 0,
      "failed to initialize SDL audio: %s", SDL_GetError());
  Assert(SDL_OpenAudio(&spec, NULL) == 0,
      "failed to open SDL audio: %s", SDL_GetError());

  sbuf_rpos = 0;
  sbuf_count = 0;
  audio_base[reg_count] = 0;
  audio_open = true;
  SDL_PauseAudio(0);
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  if (offset == reg_count * sizeof(uint32_t) && !is_write) {
    if (audio_open) SDL_LockAudio();
    audio_base[reg_count] = sbuf_count;
    if (audio_open) SDL_UnlockAudio();
    return;
  }

  if (!is_write || len != sizeof(uint32_t)) return;

  if (offset == reg_init * sizeof(uint32_t) && audio_base[reg_init] != 0) {
    init_audio_device();
  }

  if (offset == reg_count * sizeof(uint32_t)) {
    uint32_t nwrite = audio_base[reg_count];
    if (audio_open) SDL_LockAudio();
    Assert(nwrite <= CONFIG_SB_SIZE - sbuf_count,
        "audio stream buffer overflow: write=%u available=%u",
        nwrite, CONFIG_SB_SIZE - sbuf_count);
    sbuf_count += nwrite;
    audio_base[reg_count] = sbuf_count;
    if (audio_open) SDL_UnlockAudio();
  }
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
