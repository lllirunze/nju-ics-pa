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
#include <cpu/trace.h>
#include <utils.h>

#ifdef CONFIG_ETRACE

void etrace_call(word_t mstatus, word_t mtvec, vaddr_t mepc, word_t mcause, vaddr_t pc) {
  // log_write("Exception at $pc=" FMT_WORD "\n", pc);
  // log_write("          mstatus: " FMT_WORD "\n", mstatus);
  // log_write("          mtvec  : " FMT_WORD "\n", mtvec);
  // log_write("          mepc   : " FMT_WORD "\n", mepc);
  // log_write("          mcause : %d\n", mcause);
  printf("Exception at $pc = " FMT_WORD "\n", pc);
  printf("          mstatus: " FMT_WORD "\n", mstatus);
  printf("          mtvec  : " FMT_WORD "\n", mtvec);
  printf("          mepc   : " FMT_WORD "\n", mepc);
  printf("          mcause : %d\n", mcause);
}

#endif
