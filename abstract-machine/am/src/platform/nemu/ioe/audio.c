#include <am.h>
#include <klib.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

static uint32_t sbuf_wpos = 0;

void __am_audio_init() {
  sbuf_wpos = 0;
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1);
  sbuf_wpos = 0;
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  const uint8_t *data = ctl->buf.start;
  size_t remaining = (uintptr_t)ctl->buf.end - (uintptr_t)ctl->buf.start;
  uint32_t sbuf_size = inl(AUDIO_SBUF_SIZE_ADDR);

  assert(sbuf_size > 0);
  while (remaining > 0) {
    uint32_t nwrite = remaining < sbuf_size ? (uint32_t)remaining : sbuf_size;
    while ((uint32_t)inl(AUDIO_COUNT_ADDR) + nwrite > sbuf_size) {
    }

    uint32_t first = nwrite < sbuf_size - sbuf_wpos ? nwrite : sbuf_size - sbuf_wpos;
    memcpy((void *)(uintptr_t)(AUDIO_SBUF_ADDR + sbuf_wpos), data, first);
    memcpy((void *)(uintptr_t)AUDIO_SBUF_ADDR, data + first, nwrite - first);
    sbuf_wpos = (sbuf_wpos + nwrite) % sbuf_size;
    data += nwrite;
    remaining -= nwrite;

    // A write to COUNT submits this many newly written bytes to the device.
    outl(AUDIO_COUNT_ADDR, nwrite);
  }
}
