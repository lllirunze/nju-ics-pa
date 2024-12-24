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

#include <cpu/trace.h>
#include <utils.h>

#ifdef CONFIG_MTRACE

void mtrace_read(vaddr_t addr, vaddr_t pc) {
  log_write("Memory read 0x%08x at $pc=0x%08x\n", addr, pc);
}

void mtrace_write(word_t data, vaddr_t addr, vaddr_t pc) {
  log_write("Memory write %u to address 0x%08x at $pc=0x%08x\n", data, addr, pc);
}

#endif