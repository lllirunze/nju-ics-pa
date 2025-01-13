#include <common.h>
#include <trace.h>
#include <fs.h>
#include <sys/time.h>
#include "syscall.h"

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

#ifdef CONFIG_STRACE
  strace_call(c);
#endif

  switch (a[0]) {
    case SYS_exit:
      halt(a[1]);
      c->GPRx = a[1];
      break;
    case SYS_yield: 
      yield(); 
      c->GPRx = 0;
      break;
    case SYS_open:
      c->GPRx = fs_open((const char *)a[1], a[2], a[3]);
      break;
    case SYS_read:
      c->GPRx = fs_read(a[1], (void *)a[2], a[3]);
      break;
    case SYS_write:
      c->GPRx = fs_write(a[1], (void *)a[2], a[3]);
      break;
    case SYS_close:
      c->GPRx = fs_close(a[1]);
      break;
    case SYS_lseek:
      c->GPRx = fs_lseek(a[1], a[2], a[3]);
      break;
    case SYS_brk:
      /** 
       * Because Nanos-lite is a single-task OS now, 
       * any space can be used by user application.
       * So, we can just return 0.
       * In PA4, we will modify SYS_brk to implement memory allocation.
       */
      c->GPRx = 0;
      break;
    case SYS_gettimeofday:
      
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
