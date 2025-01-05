#include <proc.h>
#include <elf.h>

// 从ramdisk中`offset`偏移处的`len`字节读入到`buf`中
size_t ramdisk_read(void *buf, size_t offset, size_t len);
// 把`buf`中的`len`字节写入到ramdisk中`offset`偏移处
// size_t ramdisk_write(const void *buf, size_t offset, size_t len);
// 返回ramdisk的大小, 单位为字节
// size_t get_ramdisk_size();

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
  // TODO();

  Elf_Ehdr elf_header;
  Elf_Phdr program_header;
  ramdisk_read(&elf_header, 0, sizeof(Elf_Ehdr));
  assert(memcmp(elf_header.e_ident, ELFMAG, SELFMAG) == 0);
  assert(*((uint32_t *)&elf_header) == 0x464c457f);

  uint32_t i, phoff;
  for (i=0; i<elf_header.e_phnum; i++) {
    phoff = i*elf_header.e_phentsize + elf_header.e_phoff;
    ramdisk_read(&program_header, phoff, sizeof(Elf_Phdr));
    if (program_header.p_type == PT_LOAD) {
      void *ptr_segment = (void *)program_header.p_vaddr;
      ramdisk_read(ptr_segment, program_header.p_offset, program_header.p_filesz);
    }
  }

  return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

