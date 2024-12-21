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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/trace.h>

void init_ftrace(const char *elf_file) {
    printf("Initialize ftrace\n");

    // FILE *elf_fp = fopen(elf_file, "rb");
    // if (elf_fp == NULL) {
    //     // Assert(elf_fp, "Can not open '%s'", elf_file);
    //     printf("Can not open '%s'\n", elf_file);
    //     assert(0);
    // }
    // Log("")

}
