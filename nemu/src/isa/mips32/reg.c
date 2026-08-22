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
  "$0", "at", "v0", "v1", "a0", "a1", "a2", "a3",
  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
  "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
  "t8", "t9", "k0", "k1", "gp", "sp", "s8", "ra"
};

void isa_reg_display() {
  for (int i = 0; i < 32; i ++) {
    printf("%-3s: " FMT_WORD "%s", regs[i], cpu.gpr[i], (i % 4 == 3) ? "\n" : "  ");
  }
  printf("pc : " FMT_WORD "  hi : " FMT_WORD "  lo : " FMT_WORD "\n",
      cpu.pc, cpu.hi, cpu.lo);
  printf("status: " FMT_WORD "  cause: " FMT_WORD "  epc: " FMT_WORD "\n",
      cpu.status, cpu.cause, cpu.epc);
}

word_t isa_reg_str2val(const char *s, bool *success) {
  if (strcmp(s, "pc") == 0 || strcmp(s, "$pc") == 0) {
    *success = true;
    return cpu.pc;
  }
  if (strcmp(s, "hi") == 0) {
    *success = true;
    return cpu.hi;
  }
  if (strcmp(s, "lo") == 0) {
    *success = true;
    return cpu.lo;
  }

  const char *name = (s[0] == '$') ? s + 1 : s;
  for (int i = 0; i < 32; i ++) {
    if (strcmp(name, regs[i]) == 0) {
      *success = true;
      return cpu.gpr[i];
    }
  }
  *success = false;
  return 0;
}
