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
  pcb->max_brk = 0;
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
    if (phdr.p_vaddr + phdr.p_memsz > pcb->max_brk) {
      pcb->max_brk = phdr.p_vaddr + phdr.p_memsz;
    }
    uintptr_t va_start = ROUNDDOWN(phdr.p_vaddr, PGSIZE);
    uintptr_t va_end = ROUNDUP(phdr.p_vaddr + phdr.p_memsz, PGSIZE);
    for (uintptr_t va = va_start; va < va_end; va += PGSIZE) {
      char *pa = new_page(1);
      map(&pcb->as, (void *)va, pa, MMAP_READ | MMAP_WRITE);
      memset(pa, 0, PGSIZE);

      uintptr_t copy_start = va > phdr.p_vaddr ? va : phdr.p_vaddr;
      uintptr_t copy_end = va + PGSIZE < phdr.p_vaddr + phdr.p_filesz ?
          va + PGSIZE : phdr.p_vaddr + phdr.p_filesz;
      if (copy_start < copy_end) {
        fs_lseek(fd, phdr.p_offset + copy_start - phdr.p_vaddr, SEEK_SET);
        fs_read(fd, pa + copy_start - va, copy_end - copy_start);
      }
    }
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
  protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);
  char *ustack = new_page(STACK_SIZE / PGSIZE);
  uintptr_t ustack_va = (uintptr_t)pcb->as.area.end - STACK_SIZE;
  for (int i = 0; i < STACK_SIZE / PGSIZE; i++) {
    map(&pcb->as, (void *)(ustack_va + i * PGSIZE), ustack + i * PGSIZE,
        MMAP_READ | MMAP_WRITE);
  }

  char *sp = ustack + STACK_SIZE;
  uintptr_t vsp = (uintptr_t)pcb->as.area.end;
  int argc = 0, envc = 0;

  while (argv != NULL && argv[argc] != NULL) argc++;
  while (envp != NULL && envp[envc] != NULL) envc++;

  uintptr_t argv_addr[argc];
  uintptr_t envp_addr[envc];
  for (int i = argc - 1; i >= 0; i--) {
    size_t len = strlen(argv[i]) + 1;
    sp -= len;
    vsp -= len;
    memcpy(sp, argv[i], len);
    argv_addr[i] = vsp;
  }
  for (int i = envc - 1; i >= 0; i--) {
    size_t len = strlen(envp[i]) + 1;
    sp -= len;
    vsp -= len;
    memcpy(sp, envp[i], len);
    envp_addr[i] = vsp;
  }

  uintptr_t *args = (uintptr_t *)ROUNDDOWN(sp, sizeof(uintptr_t));
  uintptr_t v_args = ROUNDDOWN(vsp, sizeof(uintptr_t));
  args -= envc + 1;
  v_args -= (envc + 1) * sizeof(uintptr_t);
  for (int i = 0; i < envc; i++) args[i] = envp_addr[i];
  args[envc] = 0;
  args -= argc + 1;
  v_args -= (argc + 1) * sizeof(uintptr_t);
  for (int i = 0; i < argc; i++) args[i] = argv_addr[i];
  args[argc] = 0;
  *--args = argc;
  v_args -= sizeof(uintptr_t);

  pcb->cp = ucontext(&pcb->as, (Area) { pcb->stack, pcb->stack + STACK_SIZE }, (void *)entry);
  pcb->cp->gpr[2] = (uintptr_t)pcb->as.area.end;
  pcb->cp->GPRx = v_args;
}
