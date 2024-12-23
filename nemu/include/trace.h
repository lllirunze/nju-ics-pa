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

#ifndef __TARCE_H__
#define __TRACE_H__

#include <cpu/decode.h>
#include <common.h>

#define MAX_IRING_BUF 11

typedef struct iringbuf {
	char buf[128];
	struct iringbuf *next;
} iringbuf;

typedef struct functab {
	vaddr_t func_start;
	vaddr_t func_end;
	char func_name[128];
	struct functab *next;
} functab;

void insert_IRingBuf(char *p);
void display_IRingBuf();

void init_elf(const char *elf_file);
void init_ftrace(const char *ftrace_file);
void close_ftrace();
void ftrace_call(Decode *s);
void ftrace_ret(Decode *s);
void ftrace_jal(Decode *s);
void ftrace_jalr(Decode *s, uint32_t inst);

#define ftrace_write(...) IFDEF(CONFIG_FTRACE \
  do { \
    extern FILE* ftrace_fp; \
    if (ftrace_fp != NULL) { \
      fprintf(ftrace_fp, __VA_ARGS__); \
      fflush(ftrace_fp); \
    } \
  } while (0) \
)

#endif