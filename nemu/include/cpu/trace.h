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

#ifndef __CPU_TRACE_H__

#include <cpu/decode.h>
#include <device/map.h>
#include <common.h>

// ----------- itrace -----------

#define MAX_IRING_BUF 11

typedef struct iringbuf {
	char buf[128];
	struct iringbuf *next;
} iringbuf;

void insert_IRingBuf(char *p);
void display_IRingBuf();

// ----------- mtrace -----------

void mtrace_read(vaddr_t addr, vaddr_t pc);
void mtrace_write(word_t data, vaddr_t addr, vaddr_t pc);

// ----------- ftrace -----------

#define ftrace_write(...) IFDEF(CONFIG_FTRACE, \
  do { \
    extern FILE* ftrace_fp; \
    if (ftrace_fp != NULL) { \
      fprintf(ftrace_fp, __VA_ARGS__); \
      fflush(ftrace_fp); \
    } \
  } while (0) \
)

typedef struct functab {
	vaddr_t func_start;
	vaddr_t func_end;
	char func_name[128];
	struct functab *next;
} functab;

void init_elf(const char *elf_file);
void init_ftrace(const char *ftrace_file);
void close_ftrace();
void ftrace_call(Decode *s);
void ftrace_ret(Decode *s);
void ftrace_jal(Decode *s);
void ftrace_jalr(Decode *s, uint32_t inst);

// ----------- dtrace -----------

void dtrace_read(paddr_t addr, int len, IOMap *map, vaddr_t pc);
void dtrace_write(paddr_t addr, int len, word_t data, IOMap *map, vaddr_t pc);

// ----------- etrace -----------

void etrace_call(word_t mstatus, word_t mtvec, vaddr_t mepc, word_t mcause);

#endif
