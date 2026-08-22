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
#include <memory/paddr.h>

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  paddr_t pdir = (cpu.satp & 0x003fffffu) << 12;

  for (int level = 1; level >= 0; level--) {
    int vpn = (vaddr >> (12 + level * 10)) & 0x3ff;
    word_t pte = paddr_read(pdir + vpn * sizeof(word_t), sizeof(word_t));

    Assert(pte & 0x1, "invalid Sv32 PTE: va = " FMT_WORD ", level = %d, pdir = " FMT_PADDR ", pte = " FMT_WORD,
        vaddr, level, pdir, pte);
    if (pte & 0xe) {
      assert(level == 0);
      return ((pte >> 10) << 12) | (vaddr & 0xfff);
    }

    pdir = (pte >> 10) << 12;
  }

  panic("Sv32 page table walk reached an invalid leaf");
}
