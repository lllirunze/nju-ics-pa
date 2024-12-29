#include <am.h>
#include <nemu.h>
#include <stdio.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

static uint32_t buffer_position = 0;

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  // cfg->present = false;
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  // stat->count = 0;
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {

  // reference: https://github.com/nops1ed/Nemu_Riscv32/blob/pa3/abstract-machine/am/src/platform/nemu/ioe/audio.c
  
  uint8_t *audio = ctl->buf.start;
  uint32_t buffer_size = inl(AUDIO_SBUF_SIZE_ADDR);
  uint32_t cnt = inl(AUDIO_COUNT_ADDR);
  uint32_t len = ctl->buf.end - ctl->buf.start;
  
  while(len > buffer_size - cnt) { ; }

  uint8_t *AB = (uint8_t *)(uintptr_t)AUDIO_SBUF_ADDR;
  for(int i = 0; i < len; ++i) {
    AB[buffer_position] = audio[i];
    buffer_position = (buffer_position + 1) % buffer_size;  
  }

  outl(AUDIO_COUNT_ADDR, cnt + len);
}
