#include <am.h>
#include <NDL.h>
#include <string.h>
#include <time.h>

static uint32_t boot_ticks;
static int screen_w;
static int screen_h;

#define KEY_NAME(key) #key,
static const char *key_names[] = {
  "NONE",
  AM_KEYS(KEY_NAME)
};

static int keycode_from_name(const char *name) {
  for (int i = 0; i < (int)(sizeof(key_names) / sizeof(key_names[0])); i++) {
    if (strcmp(name, key_names[i]) == 0) return i;
  }
  return AM_KEY_NONE;
}

bool ioe_init() {
  if (NDL_Init(0) != 0) return false;
  NDL_OpenCanvas(&screen_w, &screen_h);
  boot_ticks = NDL_GetTicks();
  return true;
}

void ioe_read(int reg, void *buf) {
  switch (reg) {
    case AM_UART_CONFIG:
      ((AM_UART_CONFIG_T *)buf)->present = true;
      break;
    case AM_TIMER_CONFIG:
      ((AM_TIMER_CONFIG_T *)buf)->present = true;
      ((AM_TIMER_CONFIG_T *)buf)->has_rtc = true;
      break;
    case AM_TIMER_RTC: {
      time_t now = time(NULL);
      struct tm *tm = localtime(&now);
      AM_TIMER_RTC_T *rtc = buf;
      rtc->second = tm->tm_sec;
      rtc->minute = tm->tm_min;
      rtc->hour = tm->tm_hour;
      rtc->day = tm->tm_mday;
      rtc->month = tm->tm_mon + 1;
      rtc->year = tm->tm_year + 1900;
      break;
    }
    case AM_TIMER_UPTIME:
      ((AM_TIMER_UPTIME_T *)buf)->us = (uint64_t)(NDL_GetTicks() - boot_ticks) * 1000;
      break;
    case AM_INPUT_CONFIG:
      ((AM_INPUT_CONFIG_T *)buf)->present = true;
      break;
    case AM_INPUT_KEYBRD: {
      AM_INPUT_KEYBRD_T *kbd = buf;
      char event[64] = {};
      if (NDL_PollEvent(event, sizeof(event) - 1) && event[0] == 'k' &&
          (event[1] == 'd' || event[1] == 'u') && event[2] == ' ') {
        event[strcspn(event, "\r\n")] = '\0';
        kbd->keydown = event[1] == 'd';
        kbd->keycode = keycode_from_name(event + 3);
      }
      break;
    }
    case AM_GPU_CONFIG: {
      AM_GPU_CONFIG_T *cfg = buf;
      cfg->present = true;
      cfg->has_accel = false;
      cfg->width = screen_w;
      cfg->height = screen_h;
      cfg->vmemsz = 0;
      break;
    }
    case AM_GPU_STATUS:
      ((AM_GPU_STATUS_T *)buf)->ready = true;
      break;
    case AM_AUDIO_CONFIG:
    case AM_DISK_CONFIG:
    case AM_NET_CONFIG:
      break;
    default:
      break;
  }
}

void ioe_write(int reg, void *buf) {
  switch (reg) {
    case AM_UART_TX:
      putch(((AM_UART_TX_T *)buf)->data);
      break;
    case AM_GPU_FBDRAW: {
      AM_GPU_FBDRAW_T *ctl = buf;
      if (ctl->pixels != NULL && ctl->w > 0 && ctl->h > 0) {
        NDL_DrawRect(ctl->pixels, ctl->x, ctl->y, ctl->w, ctl->h);
      }
      break;
    }
    default:
      break;
  }
}
