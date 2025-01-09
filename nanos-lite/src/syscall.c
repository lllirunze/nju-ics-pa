#include <common.h>
#include <trace.h>
#include <fs.h>
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
    case SYS_write:
      /**
       * todo: sys_write (not complete)
       * On success, the number of bytes written is returned.  
       * On error, -1 is returned, and errno is set to indicate the error.
       */
      int fd = c->GPR2;
      char *buf = (char *)c->GPR3;
      size_t count = c->GPR4;
      
      c->GPRx = count;
      if (fd == 1 || fd == 2) {
        /** 
         * check the value `fd`.
         * if fd == 1 or 2, output the `len` size of `buf` 
         * to serial port (use `putch`)
        */
        size_t i;
        for (i=0; i<count; i++) {
          putch(*buf);
          buf++;
        }
      }
      else c->GPRx = -1;

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
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
