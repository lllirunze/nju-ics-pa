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
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/trace.h>
#include <memory/paddr.h>
#include <elf.h>

#ifdef CONFIG_FTRACE

static functab *functab_head = NULL;
static int level = 0;
FILE *ftrace_fp = NULL;

void init_elf(const char *elf_file) {
    
	FILE *elf_fp = fopen(elf_file, "rb");
	if (elf_fp == NULL) {
		printf("Can not open '%s'\n", elf_file);
		assert(0);
	}
	Log("Ftrace gets from %s", elf_file);

	Elf32_Ehdr elf32_ehdr;
	assert (fread(&elf32_ehdr, sizeof(elf32_ehdr), 1, elf_fp) == 1);
	if (memcmp(elf32_ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
		printf("Not a valid ELF header\n");
		fclose(elf_fp);
		assert(0);
	}

	int sh_num = elf32_ehdr.e_shnum;
	assert(sh_num > 0);
	
	fseek(elf_fp, elf32_ehdr.e_shoff, SEEK_SET);
	Elf32_Shdr *elf32_shdr = malloc(sizeof(Elf32_Shdr) * sh_num);
	assert(elf32_shdr);
	assert(fread(elf32_shdr, sizeof(Elf32_Shdr), sh_num, elf_fp) == sh_num);

	Elf32_Shdr *elf32_shdr_entry_symtab = NULL;
	Elf32_Shdr *elf32_shdr_entry_strtab = NULL;
	int i;
	for (i=0; i<sh_num; i++) {
		if (elf32_shdr[i].sh_type == SHT_SYMTAB) {
			elf32_shdr_entry_symtab = &elf32_shdr[i];
		}
		if (elf32_shdr[i].sh_type == SHT_STRTAB && i != elf32_ehdr.e_shstrndx) {
			elf32_shdr_entry_strtab = &elf32_shdr[i];
		}
	}
	assert (elf32_shdr_entry_symtab);
	assert (elf32_shdr_entry_strtab);

	assert (elf32_shdr_entry_symtab->sh_size > 0 || elf32_shdr_entry_symtab->sh_entsize > 0);
	int symtab_num = elf32_shdr_entry_symtab->sh_size / elf32_shdr_entry_symtab->sh_entsize;
	assert (symtab_num > 0);

	fseek(elf_fp, elf32_shdr_entry_symtab->sh_offset, SEEK_SET);
	Elf32_Sym *elf32_symtab = malloc(sizeof(Elf32_Sym) * symtab_num);
	assert(elf32_symtab);
	assert(fread(elf32_symtab, sizeof(Elf32_Sym), symtab_num, elf_fp) == symtab_num);

	assert(elf32_shdr_entry_strtab->sh_size > 0);
	int strtab_size = elf32_shdr_entry_strtab->sh_size;

	fseek(elf_fp, elf32_shdr_entry_strtab->sh_offset, SEEK_SET);
	char *elf32_strtab = malloc(sizeof(char) * strtab_size);
	assert(elf32_strtab);
	assert(fread(elf32_strtab, strtab_size, 1, elf_fp) == 1);

  for (i=0; i<symtab_num; i++) {
		if (ELF32_ST_TYPE(elf32_symtab[i].st_info) == STT_FUNC) {
			if (in_pmem(elf32_symtab[i].st_value)) {
				char *fname = &elf32_strtab[elf32_symtab[i].st_name];
				functab *funcsym = malloc(sizeof(functab));
				funcsym->func_start = elf32_symtab[i].st_value;
				funcsym->func_end = elf32_symtab[i].st_value + elf32_symtab[i].st_size;
				memcpy(funcsym->func_name, fname, strlen(fname));
				funcsym->next = functab_head;
				functab_head = funcsym;
			}
		}
	}

	free(elf32_shdr);
	free(elf32_symtab);
	free(elf32_strtab);

	fclose(elf_fp);
}

void init_ftrace(const char *ftrace_file) {

	ftrace_fp = stdout;
	if (ftrace_file != NULL) {
		FILE *fp = fopen(ftrace_file, "w");
		Assert(fp, "Can not open '%s'", ftrace_file);
		ftrace_fp = fp;
	}
	Log("Ftrace is written to %s", ftrace_file ? ftrace_file : "stdout");

}

void close_ftrace() {
  if (ftrace_fp != NULL) fclose(ftrace_fp);
}

char *get_function_name(vaddr_t addr) {
	char *fname = NULL;
	functab *cur = functab_head;
	while (cur != NULL) {
		if (cur->func_start <= addr && addr < cur->func_end) {
			fname = cur->func_name;
			break;
		}
		cur = cur->next;
	}
	assert(fname);
	return fname;
}

void ftrace_call(Decode *s) {
	char *fname = get_function_name(s->dnpc);
	ftrace_write("0x%08x: ", s->pc);
	int i;
	for (i=0; i<level; i++) ftrace_write("  ");
	level++;
	ftrace_write("call [%s@0x%08x]\n", fname, s->dnpc);
}

void ftrace_ret(Decode *s) {
	char *fname = get_function_name(s->pc);
	ftrace_write("0x%08x: ", s->pc);
	level--;
	int i;
	for (i=0; i<level; i++) ftrace_write("  ");
	ftrace_write("ret  [%s]\n", fname);
}

void ftrace_jal(Decode *s) {
	ftrace_call(s);
}

void ftrace_jalr(Decode *s, uint32_t inst) {
	uint32_t rd = BITS(inst, 11, 7);
	int32_t imm = SEXT(BITS(inst, 31, 20), 12);
	if (inst == 0x00008067) ftrace_ret(s);
	else if (rd == 1) ftrace_call(s);
	else if (rd == 1 && imm == 0) ftrace_call(s);
}

#endif
