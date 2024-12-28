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

#ifdef CONFIG_DTRACE

void dtrace_read(paddr_t addr, int len, IOMap *map, vaddr_t pc) {
  log_write("Device:  read \"%s\" at address " FMT_PADDR "(len=%d) at $pc=" FMT_WORD "\n", map->name, addr, len, pc);
}

void dtrace_write(paddr_t addr, int len, word_t data, IOMap *map, vaddr_t pc) {
  log_write("Device: write \"%s\" at address " FMT_PADDR "(len=%d) with data=" FMT_DATA " at $pc=" FMT_WORD "\n", map->name, addr, len, data, pc);
}

#endif
