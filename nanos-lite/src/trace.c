#include <common.h>
#include <fs.h>
#include <trace.h>
#include "syscall.h"

#ifdef CONFIG_STRACE

const char* syscall_names[] = {
  "SYS_exit",   "SYS_yield",  "SYS_open",   "SYS_read",
  "SYS_write",  "SYS_kill",   "SYS_getpid", "SYS_close",
  "SYS_lseek",  "SYS_brk",    "SYS_fstat",  "SYS_time",
  "SYS_signal", "SYS_execve", "sis_fork",   "SYS_link",
  "SYS_unlink", "SYS_wait",   "SYS_times",  "SYS_gettimeofday",
};
const int sys_size = sizeof(syscall_names)/sizeof(syscall_names[0]);

void strace_call(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  switch (check_sys_idx(a[0])) {
    case SYS_open:
      Log("System call: [%s, %s]", sys_name(a[0]), (const char *)a[1]);
      break;
    case SYS_close:
      Log("System call: [%s, %s]", sys_name(a[0]), fs_name(a[1]));
      break;
    case SYS_read:
    case SYS_write:
      Log("System call: [%s, %s, %d bytes]", sys_name(a[0]), fs_name(a[1]), a[3]);
      break;
    default:
      Log("System call: [%s]" , sys_name(a[0]));
      break;
  }
}

#endif