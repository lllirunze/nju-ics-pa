#include <common.h>
#include <trace.h>
#include "syscall.h"

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

#ifdef CONFIG_STRACE
  strace_call(c);
#endif

  switch (a[0]) {
    case SYS_exit:
      halt(0);
      break;
    case SYS_yield: 
      yield(); 
      c->GPRx = 0;
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
