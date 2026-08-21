/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
***************************************************************************************/

#include <elf.h>
#include <ftrace.h>
#include <utils.h>

typedef struct {
  vaddr_t start;
  uint32_t size;
  const char *name;
} FunctionSymbol;

static FunctionSymbol *functions = NULL;
static size_t nr_functions = 0;
static int call_depth = 0;

static const char *find_function(vaddr_t addr) {
  for (size_t i = 0; i < nr_functions; i++) {
    uint64_t end = (uint64_t)functions[i].start + functions[i].size;
    if (addr >= functions[i].start && (uint64_t)addr < end) {
      return functions[i].name;
    }
  }
  return "???";
}

static void check_section(const Elf32_Shdr *section, size_t elf_size) {
  Assert(section->sh_offset <= elf_size && section->sh_size <= elf_size - section->sh_offset,
      "ELF section is outside the file");
}

void init_ftrace(const char *elf_file) {
  Assert(elf_file != NULL, "ftrace requires an ELF file passed with -f ELF_FILE");

  FILE *fp = fopen(elf_file, "rb");
  Assert(fp != NULL, "Can not open ftrace ELF file '%s'", elf_file);
  Assert(fseek(fp, 0, SEEK_END) == 0, "Can not seek ftrace ELF file '%s'", elf_file);
  long file_size = ftell(fp);
  Assert(file_size >= (long)sizeof(Elf32_Ehdr), "'%s' is not a valid ELF32 file", elf_file);
  Assert(fseek(fp, 0, SEEK_SET) == 0, "Can not seek ftrace ELF file '%s'", elf_file);

  size_t elf_size = (size_t)file_size;
  uint8_t *elf = malloc(elf_size);
  Assert(elf != NULL, "Can not allocate memory for ftrace ELF file");
  Assert(fread(elf, elf_size, 1, fp) == 1, "Can not read ftrace ELF file '%s'", elf_file);
  fclose(fp);

  Elf32_Ehdr *header = (Elf32_Ehdr *)elf;
  Assert(memcmp(header->e_ident, ELFMAG, SELFMAG) == 0 &&
      header->e_ident[EI_CLASS] == ELFCLASS32 && header->e_ident[EI_DATA] == ELFDATA2LSB,
      "'%s' is not a little-endian ELF32 file", elf_file);
  Assert(header->e_shentsize == sizeof(Elf32_Shdr) && header->e_shnum > 0 &&
      header->e_shoff <= elf_size && header->e_shnum <= (elf_size - header->e_shoff) / sizeof(Elf32_Shdr),
      "'%s' has invalid section headers", elf_file);

  Elf32_Shdr *sections = (Elf32_Shdr *)(elf + header->e_shoff);
  Elf32_Shdr *symtab = NULL;
  for (int i = 0; i < header->e_shnum; i++) {
    check_section(&sections[i], elf_size);
    if (sections[i].sh_type == SHT_SYMTAB) {
      symtab = &sections[i];
      break;
    }
  }
  Assert(symtab != NULL && symtab->sh_entsize == sizeof(Elf32_Sym) &&
      symtab->sh_link < header->e_shnum, "'%s' has no valid symbol table", elf_file);

  Elf32_Shdr *strtab = &sections[symtab->sh_link];
  check_section(strtab, elf_size);
  Assert(strtab->sh_type == SHT_STRTAB, "'%s' has no valid symbol string table", elf_file);

  Elf32_Sym *symbols = (Elf32_Sym *)(elf + symtab->sh_offset);
  size_t nr_symbols = symtab->sh_size / sizeof(Elf32_Sym);
  functions = malloc(nr_symbols * sizeof(*functions));
  Assert(functions != NULL, "Can not allocate ftrace symbol table");
  const char *strings = (const char *)(elf + strtab->sh_offset);

  for (size_t i = 0; i < nr_symbols; i++) {
    Elf32_Sym *symbol = &symbols[i];
    if (ELF32_ST_TYPE(symbol->st_info) != STT_FUNC || symbol->st_size == 0 ||
        symbol->st_name >= strtab->sh_size || strings[symbol->st_name] == '\0') {
      continue;
    }
    functions[nr_functions++] = (FunctionSymbol) {
      .start = symbol->st_value,
      .size = symbol->st_size,
      .name = strings + symbol->st_name,
    };
  }

  Log("ftrace: loaded %zu function symbols from %s", nr_functions, elf_file);
}

void ftrace_call(vaddr_t pc, vaddr_t target) {
  log_write(FMT_WORD ": %*scall [%s@" FMT_WORD "]\n",
      pc, call_depth * 2, "", find_function(target), target);
  call_depth++;
}

void ftrace_ret(vaddr_t pc) {
  if (call_depth > 0) {
    call_depth--;
  }
  log_write(FMT_WORD ": %*sret  [%s]\n", pc, call_depth * 2, "", find_function(pc));
}
