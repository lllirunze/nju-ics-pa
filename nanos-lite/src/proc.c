#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
static PCB *foreground = NULL;
static char *hello_argv[] = { "/bin/hello", NULL };
static char *nterm_argv[] = { "/bin/nterm", NULL };

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area) { pcb->stack, pcb->stack + STACK_SIZE }, entry, arg);
}

void switch_boot_pcb() {
  current = &pcb_boot;
}

bool foreground_input_owner(void) {
  return current == foreground;
}

bool handle_foreground_key(int keycode, bool keydown) {
  if (!keydown) return false;

  if (keycode == AM_KEY_F1) {
    foreground = &pcb[0];
    Log("Foreground process: hello (F2 returns to NTerm/PAL)");
    return true;
  }
  if (keycode == AM_KEY_F2) {
    foreground = &pcb[1];
    Log("Foreground process: NTerm/PAL (F1 selects hello)");
    return true;
  }
  return false;
}

void init_proc() {
  switch_boot_pcb();

  Log("Initializing processes...");

  context_uload(&pcb[0], "/bin/hello", hello_argv, NULL);
  context_uload(&pcb[1], "/bin/nterm", nterm_argv, NULL);
  foreground = &pcb[1];

}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = current == &pcb[0] ? &pcb[1] : &pcb[0];
  return current->cp;
}
