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

#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  int reg_num = ARRLEN(cpu.gpr);
  int i;
  for (i = 0; i < reg_num; i++) {
    printf("%s\t0x%08x\t%u\n", reg_name(i), gpr(i), gpr(i));
  }
  printf("pc\t0x%08x\t%u\n", cpu.pc, cpu.pc);
}

word_t isa_reg_str2val(const char *s, bool *success) {
  int reg_num = ARRLEN(cpu.gpr);
  int i;
  for (i=0; i<reg_num; i++) {
    if (strcmp(s, reg_name(i)) == 0) {
      return gpr(i);
    }
  }
  if (strcmp(s, "pc") == 0) return cpu.pc;
  *success = false;
  return 0;
}

word_t *isa_reg_num2csr(word_t num) {
  switch (num) {
    case 0x300: return &cpu.sr.mstatus;
    case 0x305: return &cpu.sr.mtvec;
    case 0x341: return &cpu.sr.mepc;
    case 0x342: return &cpu.sr.mcause;
    default:
      panic("Invalid CSR address!");
      break;
  }
  return NULL;
}
