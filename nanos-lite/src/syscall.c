#include <common.h>
#include <trace.h>
#include <fs.h>
#include <device.h>
#include <proc.h>
#include <loader.h>
#include "syscall.h"

// void naive_uload(PCB *pcb, const char *filename);

void sys_exit(int status);
int sys_yield();
int sys_brk();
int sys_execve(const char *fname, char * const argv[], char *const envp[]);

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
      sys_exit(a[1]);
      break;
    case SYS_yield: 
      c->GPRx = sys_yield();
      break;
    case SYS_open:
      c->GPRx = fs_open((char *)a[1], a[2], a[3]);
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
      c->GPRx = sys_brk();
      break;
    case SYS_execve:
      c->GPRx = sys_execve((void*)a[1], (void*)a[2], (void*)a[3]);
      while(1);
      break;
    case SYS_gettimeofday:
      c->GPRx = timer_read((struct timeval *)a[1], (struct timezone *)a[2]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

void sys_exit(int status) {
  sys_execve("/bin/nterm", (char *const[1]){ NULL }, (char *const[1]) { NULL });
  halt(status);
  return;
}

int sys_yield() {
  yield(); 
  return 0;
}

int sys_brk() {
  /** 
   * Because Nanos-lite is a single-task OS now, 
   * any space can be used by user application.
   * So, we can just return 0.
   * In PA4, we will modify SYS_brk to implement memory allocation.
   */
  return 0;
}

int sys_execve(const char *fname, char *const argv[], char *const envp[]) {
  // naive_uload(NULL, fname);
  // return -1;
  
  context_uload(current, fname, argv, envp);
  switch_boot_pcb();
  yield();
  return -1;
}