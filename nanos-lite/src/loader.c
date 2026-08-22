#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr;
  (void)pcb;
  int fd = fs_open(filename, 0, 0);

  fs_read(fd, &ehdr, sizeof(ehdr));
  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
  assert(ehdr.e_machine == EM_RISCV);
  assert(ehdr.e_phentsize == sizeof(Elf_Phdr));

  for (int i = 0; i < ehdr.e_phnum; i++) {
    Elf_Phdr phdr;
    fs_lseek(fd, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET);
    fs_read(fd, &phdr, sizeof(phdr));
    if (phdr.p_type != PT_LOAD) {
      continue;
    }

    assert(phdr.p_memsz >= phdr.p_filesz);
    fs_lseek(fd, phdr.p_offset, SEEK_SET);
    fs_read(fd, (void *)phdr.p_vaddr, phdr.p_filesz);
    memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0,
        phdr.p_memsz - phdr.p_filesz);
  }

  fs_close(fd);
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  uintptr_t entry = loader(pcb, filename);
  char *ustack = new_page(STACK_SIZE / PGSIZE);
  char *sp = ustack + STACK_SIZE;
  int argc = 0, envc = 0;

  while (argv != NULL && argv[argc] != NULL) argc++;
  while (envp != NULL && envp[envc] != NULL) envc++;

  uintptr_t argv_addr[argc];
  uintptr_t envp_addr[envc];
  for (int i = argc - 1; i >= 0; i--) {
    size_t len = strlen(argv[i]) + 1;
    sp -= len;
    memcpy(sp, argv[i], len);
    argv_addr[i] = (uintptr_t)sp;
  }
  for (int i = envc - 1; i >= 0; i--) {
    size_t len = strlen(envp[i]) + 1;
    sp -= len;
    memcpy(sp, envp[i], len);
    envp_addr[i] = (uintptr_t)sp;
  }

  uintptr_t *args = (uintptr_t *)ROUNDDOWN(sp, sizeof(uintptr_t));
  args -= envc + 1;
  for (int i = 0; i < envc; i++) args[i] = envp_addr[i];
  args[envc] = 0;
  args -= argc + 1;
  for (int i = 0; i < argc; i++) args[i] = argv_addr[i];
  args[argc] = 0;
  *--args = argc;

  pcb->cp = ucontext(&pcb->as, (Area) { pcb->stack, pcb->stack + STACK_SIZE }, (void *)entry);
  pcb->cp->gpr[2] = (uintptr_t)(ustack + STACK_SIZE);
  pcb->cp->GPRx = (uintptr_t)args;
}
