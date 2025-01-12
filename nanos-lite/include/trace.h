#ifndef __TRACE_H__
#define __TRACE_H__

#include <common.h>

// ----------- strace -----------

#ifdef CONFIG_STRACE

void strace_call(Context *c);

static inline int check_sys_idx(int idx) {
  extern const int sys_size;
  assert(idx >= 0 && idx < sys_size);
  return idx;
}

static inline const char* sys_name(int idx) {
  extern const char* syscall_names[];
  return syscall_names[check_sys_idx(idx)];
}

#endif

#endif
