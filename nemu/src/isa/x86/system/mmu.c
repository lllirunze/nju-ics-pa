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
  const word_t PTE_P = 0x001;
  const word_t PTE_W = 0x002;
  const word_t PTE_A = 0x020;
  const word_t PTE_D = 0x040;
  const word_t PTE_ADDR = 0xfffff000u;

  paddr_t pde_addr = (cpu.cr3 & PTE_ADDR) + (((vaddr >> 22) & 0x3ff) << 2);
  word_t pde = paddr_read(pde_addr, 4);
  if (!(pde & PTE_P)) {
    cpu.cr2 = vaddr;
    panic("x86 page fault: PDE is not present for vaddr=" FMT_WORD, vaddr);
  }
  paddr_write(pde_addr, 4, pde | PTE_A);

  paddr_t pte_addr = (pde & PTE_ADDR) + (((vaddr >> 12) & 0x3ff) << 2);
  word_t pte = paddr_read(pte_addr, 4);
  if (!(pte & PTE_P) || (type == MEM_TYPE_WRITE && !(pte & PTE_W))) {
    cpu.cr2 = vaddr;
    panic("x86 page fault: invalid PTE for vaddr=" FMT_WORD, vaddr);
  }
  paddr_write(pte_addr, 4, pte | PTE_A | (type == MEM_TYPE_WRITE ? PTE_D : 0));
  return (pte & PTE_ADDR) | (vaddr & 0xfff);
}

void x86_mmu_selftest() {
  const paddr_t pd = 0x3000, pt = 0x4000, page = 0x5000;
  const vaddr_t va = 0x00403004;
  word_t saved_cr0 = cpu.cr0;
  paddr_t saved_cr3 = cpu.cr3;

  paddr_write(pd + 1 * 4, 4, pt | 0x3);
  paddr_write(pt + 3 * 4, 4, page | 0x3);
  cpu.cr3 = pd;
  cpu.cr0 |= 0x80000000u;

  assert(isa_mmu_translate(va, 4, MEM_TYPE_READ) == page + 4);
  assert(paddr_read(pd + 4, 4) & 0x20);
  assert(paddr_read(pt + 12, 4) & 0x20);
  assert(isa_mmu_translate(va, 4, MEM_TYPE_WRITE) == page + 4);
  assert(paddr_read(pt + 12, 4) & 0x40);

  cpu.cr0 = saved_cr0;
  cpu.cr3 = saved_cr3;
}
