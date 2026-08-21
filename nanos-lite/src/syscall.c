#include <common.h>
#include <fs.h>
#include <sys/time.h>
#include "syscall.h"

int mm_brk(uintptr_t brk);
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
    case SYS_yield:
      yield();
      c->GPRx = 0;
      break;
    case SYS_exit:
      halt(a[1]);
      break;
    case SYS_open:
      c->GPRx = fs_open((const char *)a[1], a[2], a[3]);
      break;
    case SYS_read:
      c->GPRx = fs_read(a[1], (void *)a[2], a[3]);
      break;
    case SYS_write:
      c->GPRx = fs_write(a[1], (const void *)a[2], a[3]);
      break;
    case SYS_lseek:
      c->GPRx = fs_lseek(a[1], a[2], a[3]);
      break;
    case SYS_close:
      c->GPRx = fs_close(a[1]);
      break;
    case SYS_brk:
      c->GPRx = mm_brk(a[1]);
      break;
    case SYS_gettimeofday: {
      struct timeval *tv = (struct timeval *)a[1];
      uint64_t us = io_read(AM_TIMER_UPTIME).us;
      tv->tv_sec = us / 1000000;
      tv->tv_usec = us % 1000000;
      c->GPRx = 0;
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
