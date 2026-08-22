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
#include <memory/paddr.h>
#include <memory/vaddr.h>

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  const word_t VPN2_MASK = 0xffffe000u;
  const word_t ASID_MASK = 0x000000ffu;
  const word_t ENTRYLO_G = 0x1;
  const word_t ENTRYLO_V = 0x2;
  const word_t ENTRYLO_D = 0x4;

  for (int i = 0; i < ARRLEN(cpu.tlb); i ++) {
    mips32_TLBEntry *entry = &cpu.tlb[i];
    bool global = (entry->entrylo0 & ENTRYLO_G) && (entry->entrylo1 & ENTRYLO_G);
    if ((entry->entryhi & VPN2_MASK) != (vaddr & VPN2_MASK) ||
        (!global && (entry->entryhi & ASID_MASK) != (cpu.entryhi & ASID_MASK))) {
      continue;
    }

    word_t lo = (vaddr & 0x1000) ? entry->entrylo1 : entry->entrylo0;
    if (!(lo & ENTRYLO_V) || (type == MEM_TYPE_WRITE && !(lo & ENTRYLO_D))) {
      cpu.badvaddr = vaddr;
      panic("mips32 TLB invalid/modification fault at vaddr=" FMT_WORD, vaddr);
    }
    return ((lo >> 6) << 12) | (vaddr & 0xfff);
  }

  cpu.badvaddr = vaddr;
  panic("mips32 TLB refill fault at vaddr=" FMT_WORD, vaddr);
}

void mips32_tlb_selftest() {
  const vaddr_t va = 0x00402004;
  const paddr_t pa = 0x80005004;
  mips32_TLBEntry saved = cpu.tlb[0];
  word_t saved_entryhi = cpu.entryhi;

  cpu.entryhi = va & 0xffffe000u;
  cpu.tlb[0].entryhi = cpu.entryhi;
  cpu.tlb[0].entrylo0 = ((pa & 0xfffff000u) >> 6) | 0x7;
  cpu.tlb[0].entrylo1 = 0;
  assert(isa_mmu_translate(va, 4, MEM_TYPE_READ) == pa);
  assert(isa_mmu_translate(va, 4, MEM_TYPE_WRITE) == pa);

  cpu.tlb[0] = saved;
  cpu.entryhi = saved_entryhi;
}
