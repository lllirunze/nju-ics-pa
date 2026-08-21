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
#include <utils.h>

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  cpu.mepc = epc;
  cpu.mcause = NO;

  // NEMU currently executes in M-mode. Preserve MIE in MPIE and record MPP.
  word_t mstatus = cpu.mstatus;
  cpu.mstatus &= ~((word_t)0x3 << 11 | (word_t)1 << 7 | (word_t)1 << 3);
  cpu.mstatus |= (word_t)3 << 11;
  cpu.mstatus |= ((mstatus >> 3) & 1) << 7;

  IFDEF(CONFIG_ETRACE,
      log_write("etrace: raise cause=" FMT_WORD " epc=" FMT_WORD " mtvec=" FMT_WORD "\n",
          NO, epc, cpu.mtvec));

  return cpu.mtvec;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
