#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
void __libc_init_array(void);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = *args;
  char **argv = (char **)(args + 1);
  char **envp = argv + argc + 1;
  environ = envp;
  __libc_init_array();
  exit(main(argc, argv, envp));
  assert(0);
}
