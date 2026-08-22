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

#ifndef __ISA_MIPS32_H__
#define __ISA_MIPS32_H__

#include <common.h>

typedef struct {
  word_t entryhi;
  word_t entrylo0;
  word_t entrylo1;
} mips32_TLBEntry;

typedef struct {
  word_t gpr[32];
  word_t index;
  word_t entrylo0;
  word_t entrylo1;
  word_t status;
  word_t lo;
  word_t hi;
  vaddr_t badvaddr;
  word_t cause;
  vaddr_t epc;
  word_t entryhi;
  vaddr_t pc;
  bool INTR;
  mips32_TLBEntry tlb[16];
} mips32_CPU_state;

// decode
typedef struct {
  uint32_t inst;
} mips32_ISADecodeInfo;

#define isa_mmu_check(vaddr, len, type) \
  ((((vaddr) & 0xe0000000u) == 0x80000000u || ((vaddr) & 0xe0000000u) == 0xa0000000u) ? MMU_DIRECT : MMU_TRANSLATE)

#endif
