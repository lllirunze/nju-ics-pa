#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {

  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case -1: ev.event = EVENT_YIELD; break;
      case SYS_exit:
      case SYS_yield:
      case SYS_open:
      case SYS_read:
      case SYS_write:
      case SYS_close:
      case SYS_lseek:
      case SYS_brk:
      case SYS_execve:
      case SYS_gettimeofday:
        ev.event = EVENT_SYSCALL;
        break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);

    if (ev.event == EVENT_YIELD || ev.event == EVENT_SYSCALL) {
      c->mepc += 4;
    }
  
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {

  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {

}
