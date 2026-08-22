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
#include <memory/vaddr.h>

word_t isa_raise_intr(word_t NO, vaddr_t ret_addr) {
  vaddr_t gate = cpu.idtr.base + NO * 8;
  assert(NO * 8 + 7 <= cpu.idtr.limit);
  uint32_t low = vaddr_read(gate, 4);
  uint32_t high = vaddr_read(gate + 4, 4);
  vaddr_t handler = (low & 0xffff) | (high & 0xffff0000);

  cpu.esp -= 4;
  vaddr_write(cpu.esp, 4, cpu.eflags);
  cpu.esp -= 4;
  vaddr_write(cpu.esp, 4, 0);  // The teaching subset uses a flat code segment.
  cpu.esp -= 4;
  vaddr_write(cpu.esp, 4, ret_addr);
  cpu.eflags &= ~((word_t)1 << 9);  // IF
  return handler;
}

word_t isa_query_intr() {
  if (cpu.INTR && (cpu.eflags & ((word_t)1 << 9))) {
    cpu.INTR = false;
    return 32;
  }
  return INTR_EMPTY;
}
