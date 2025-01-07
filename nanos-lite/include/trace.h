#ifndef __TRACE_H__
#define __TRACE_H__

#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <debug.h>

// ----------- strace -----------

#ifdef CONFIG_STRACE

const char* syscall_names[] = {
  "SYS_exit",   "SYS_yield",  "SYS_open",   "SYS_read",
  "SYS_write",  "SYS_kill",   "SYS_getpid", "SYS_close",
  "SYS_lseek",  "SYS_brk",    "SYS_fstat",  "SYS_time",
  "SYS_signal", "SYS_execve", "sis_fork",   "SYS_link",
  "SYS_unlink", "SYS_wait",   "SYS_times",  "SYS_gettimeofday",
};

void strace_call(Context *c) {
  Log("System call: [%s]" , syscall_names[c->GPR1]);
}

#endif

#endif
