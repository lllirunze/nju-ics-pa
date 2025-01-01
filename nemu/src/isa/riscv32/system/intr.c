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
  /**
   * TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   * 
   * Attention:
   * 1. PA don't care about privileged switching and you don't care about that.
   * 2. You need to use isa_raise_intr() in the traps instead of exception.
   */
  // printf("mcause: %d\n", NO);
  cpu.sr.mcause = NO;
  cpu.sr.mepc = epc;

  /**
   * todo: when we find this is ecall instruction, 
   * we need to add 4 to mepc.
   * otherwise, mepc doesn't need to add 4.
   */
  cpu.sr.mepc += 4;

  return cpu.sr.mtvec;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
