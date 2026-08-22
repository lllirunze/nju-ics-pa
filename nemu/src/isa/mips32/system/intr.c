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

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  cpu.epc = epc;
  cpu.cause = (cpu.cause & ~((word_t)0x1f << 2)) | ((NO & 0x1f) << 2);
  cpu.status |= (word_t)0x2;  // Status.EXL

  return 0x80000180;
}

word_t isa_query_intr() {
  if (cpu.INTR && (cpu.status & 0x1) && !(cpu.status & 0x2)) {
    cpu.INTR = false;
    return 0;
  }
  return INTR_EMPTY;
}
