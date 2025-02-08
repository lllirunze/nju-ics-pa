#include <proc.h>
#include <fs.h>
#include <ramdisk.h>
#include <loader.h>
#include <elf.h>

#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_X86__)
# define EXPECT_TYPE EM_386
#elif defined(__ISA_RISCV32__)
# define EXPECT_TYPE EM_RISCV
#elif defined(__ISA_MIPS32__)
# define EXPECT_TYPE EM_MIPS
#else
# error Unsupported ISA
#endif

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

uintptr_t loader(PCB *pcb, const char *filename) {
  /** 
   * todo: now we can ignore these parameters.
   * 
   * We can ignore pcb because we don't need to use it.
   * We can also ignore filename because ramdisk only have one file.
   * In the next phase, after we implement file system, we will use filename.
   * 
   */

  // int fd = fs_open(filename, 0, 0);
  // assert(fd != -1);

  // Elf_Ehdr ehdr;
  // assert(fs_read(fd, &ehdr, sizeof(Elf_Ehdr)) > 0);
  
  // // check valid elf
  // assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
  // // check ISA type in elf
  // assert(ehdr.e_machine == EXPECT_TYPE);

  // Elf_Phdr phdr;
  // size_t i, phoff;
  // for (i=0; i<ehdr.e_phnum; i++) {
  //   phoff = i*ehdr.e_phentsize + ehdr.e_phoff;
  //   fs_lseek(fd, phoff, SEEK_SET);
  //   fs_read(fd, &phdr, sizeof(Elf_Phdr));
  //   if (phdr.p_type == PT_LOAD) {
  //     fs_lseek(fd, phdr.p_offset, SEEK_SET);
  //     fs_read(fd, (void*)phdr.p_vaddr, phdr.p_filesz);
  //     memset((void*)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
  //   }
  // }

  // return ehdr.e_entry;


  int fd = fs_open(filename, 0, 0);
  Elf_Ehdr elf_ehdr;
  fs_read(fd, &elf_ehdr, sizeof elf_ehdr);
  assert(*(uint32_t*)(elf_ehdr.e_ident) == 0x464c457f);
  assert(elf_ehdr.e_machine == EXPECT_TYPE);
  assert(elf_ehdr.e_phnum);
  Elf_Phdr elf_phdr[elf_ehdr.e_phnum];
  fs_lseek(fd, elf_ehdr.e_phoff, SEEK_SET);
  fs_read(fd, &elf_phdr, elf_ehdr.e_phentsize * elf_ehdr.e_phnum);
  for (size_t i = 0; i < elf_ehdr.e_phnum; i++) {
    if (elf_phdr[i].p_type == PT_LOAD) {
      void *loader_ptr = (void*)elf_phdr[i].p_vaddr;
      fs_lseek(fd, elf_phdr[i].p_offset, SEEK_SET);
      fs_read(fd, loader_ptr, elf_phdr[i].p_filesz);
      memset(loader_ptr + elf_phdr[i].p_filesz, 0, elf_phdr[i].p_memsz - elf_phdr[i].p_filesz);
    }
  }
  fs_close(fd);
  return elf_ehdr.e_entry;

}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area){pcb->stack, pcb->stack+STACK_SIZE}, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  // Area area = {.start=pcb->stack, .end=pcb->stack + STACK_SIZE};
  // uintptr_t entry = loader(pcb, filename);
  // Log("Jump to entry = %p", entry);
  // pcb->cp = ucontext(NULL, area, (void *)entry);
  // pcb->cp->GPRx = (uintptr_t)heap.end;




  // uintptr_t* ustack = (uintptr_t*)(new_page(8) + 8 * PGSIZE);
  // Area kstack;
  // kstack.start = pcb; // this is for PCB on stack, processed by kernel
  // kstack.end = &pcb->stack[sizeof(pcb->stack)];
  // uintptr_t entry = loader(pcb, filename);
  // pcb->cp = ucontext(NULL, kstack, (void*)entry);
  
  // // Set argv, envp
  // int argv_count = 0;
  // int envp_count = 0;
  // char* _argv[20] = {0};
  // char* _envp[20] = {0}; // tmp solution
  // // collect arg count
  // if (argv) while (argv[argv_count]) argv_count ++;
  // if (envp) while (envp[envp_count]) envp_count ++;
  // // copy strings
  // for (int i = 0; i < envp_count; ++ i) {
  //   ustack -= strlen(envp[i]) + 1;
  //   strcpy((char*)ustack, envp[i]);
  //   _envp[i] = (char*)ustack;
  // }
  // for (int i = 0; i < argv_count; ++ i) {
  //   ustack -= strlen(argv[i]) + 1;
  //   strcpy((char*)ustack, argv[i]);
  //   _argv[i] = (char*)ustack;
  // }
  // // copy argv table
  // size_t envp_size = sizeof(char*) * (envp_count + 1);
  // size_t argv_size = sizeof(char*) * (argv_count + 1);
  // ustack -= envp_size; // there should be a null at the end 
  // memcpy((void*) ustack, _envp, envp_size);
  // ustack -= argv_size;
  // memcpy((void*) ustack, _argv, argv_size);
  // // set argc
  // ustack -= sizeof(uintptr_t);
  // *(uintptr_t *)ustack = argv_count;
  // // set stack pos
  // pcb->cp->GPRx = ustack;




  void *u_heap = new_page(8);
  void *u_heap_end = (char *)u_heap + 8 * PGSIZE;
  size_t string_size = 0;
  int argc = 0;
  for (; argv[argc]; argc++) { string_size += strlen(argv[argc]) + 1; }
  int envpc = 0;
  for (; envp[envpc]; envpc++) { string_size += strlen(envp[envpc]) + 1; }
  char *string_area = (char*)u_heap_end - string_size;
  char **u_envp = (char**)string_area - envpc - 1;
  char **u_argv = (char**)u_envp - argc - 1;
  int *u_argc = (int*)u_argv - 1;
  *u_argc = argc;
  for (int i = 0; i < argc; i++) {
    strcpy(string_area, argv[i]);
    u_argv[i] = string_area;
    string_area += strlen(string_area) + 1;
  }
  u_argv[argc] = NULL;
  for (int i = 0; i < envpc; i++) {
    strcpy(string_area, envp[i]);
    u_envp[i] = string_area;
    string_area += strlen(string_area) + 1;
  }
  u_envp[envpc] = NULL;
  uintptr_t entry = loader(pcb, filename);
  pcb->cp = ucontext(NULL, (Area){.start=pcb->stack, .end=pcb+1}, (void*)entry);
  pcb->cp->GPRx = (uintptr_t)u_argc;
  Log("Load %s @ %p, user stack top @ %p", filename, entry, (void*)pcb->cp->GPRx);

}
