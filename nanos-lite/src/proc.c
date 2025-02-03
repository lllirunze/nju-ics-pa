#include <proc.h>
#include <loader.h>
#include <memory.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

// void naive_uload(PCB *pcb, const char *filename);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    // Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    
    for (int volatile i = 0; i < 10000000; i++) ;
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (uintptr_t)arg, j);
    
    j ++;
    yield();
  }
}

void init_proc() {

  // implement context switching in Nanos-lite
  // context_kload(&pcb[0], hello_fun, "hello nanos-lite-1");
  // context_kload(&pcb[1], hello_fun, "hello nanos-lite-2");
  // switch_boot_pcb();
  
  // implement multiple programs
  // context_kload(&pcb[0], hello_fun, "hello nanos-lite");
  // context_uload(&pcb[0], "/bin/hello");
  // context_uload(&pcb[1], "/bin/pal");

  // implement multiple programs with parameters
  char* argv[] = { "--skip", NULL };
  char* envp[] = { NULL };
  // context_kload(&pcb[0], hello_fun, "hello nanos-lite");
  context_uload(&pcb[1], "/bin/pal", argv, envp);
  switch_boot_pcb();

  Log("Initializing processes...");

  // naive_uload(NULL, "/bin/pal");
}

Context* schedule(Context *prev) {
  // return NULL;
  current->cp = prev;
  // printf("switch to %d\n", (current == &pcb[0] ? 1 : 0));
  // current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  current = &pcb[1];
  return current->cp;
}
