#include <proc.h>
#include <fs.h>
#include <ramdisk.h>
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

static uintptr_t loader(PCB *pcb, const char *filename) {
  /** 
   * todo: now we can ignore these parameters.
   * 
   * We can ignore pcb because we don't need to use it.
   * We can also ignore filename because ramdisk only have one file.
   * In the next phase, after we implement file system, we will use filename.
   * 
   */

  int fd = fs_open(filename, 0, 0);
  assert(fd != -1);

  Elf_Ehdr ehdr;
  assert(fs_read(fd, &ehdr, sizeof(Elf_Ehdr)) > 0);
  
  // check valid elf
  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
  // check ISA type in elf
  assert(ehdr.e_machine == EXPECT_TYPE);

  Elf_Phdr phdr;
  size_t i, phoff;
  for (i=0; i<ehdr.e_phnum; i++) {
    phoff = i*ehdr.e_phentsize + ehdr.e_phoff;
    fs_lseek(fd, phoff, SEEK_SET);
    fs_read(fd, &phdr, sizeof(Elf_Phdr));
    if (phdr.p_type == PT_LOAD) {
      fs_lseek(fd, phdr.p_offset, SEEK_SET);
      fs_read(fd, (void*)phdr.p_vaddr, phdr.p_filesz);
      memset((void*)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
    }
  }

  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

