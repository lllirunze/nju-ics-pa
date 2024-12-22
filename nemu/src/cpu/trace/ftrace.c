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
#include <elf.h>

void init_ftrace(const char *elf_file) {
    
    FILE *elf_fp = fopen(elf_file, "rb");
    if (elf_fp == NULL) {
        printf("Can not open '%s'\n", elf_file);
        assert(0);
    }
    Log("FTrace gets from %s", elf_file);

    // read elf header
    Elf32_Ehdr elf32_ehdr;
    assert (fread(&elf32_ehdr, sizeof(elf32_ehdr), 1, elf_fp) == 1);
    if (memcmp(elf32_ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
        printf("Not a valid ELF header\n");
        fclose(elf_fp);
        assert(0);
    }

    // get the number of section headers
    int sh_num = elf32_ehdr.e_shnum;
    assert(sh_num > 0);
    printf("Number of section headers: %d\n", sh_num);
    
    // read section headers
    fseek(elf_fp, elf32_ehdr.e_shoff, SEEK_SET);
    Elf32_Shdr *elf32_shdr = malloc(sizeof(Elf32_Shdr) * sh_num);
    assert(elf32_shdr);
    assert(fread(elf32_shdr, sizeof(Elf32_Shdr), sh_num, elf_fp) == sh_num);

    // todo: find symbol table and string table
    Elf32_Shdr *elf32_shdr_entry_symtab = NULL;
    Elf32_Shdr *elf32_shdr_entry_strtab = NULL;
    int i;
    for (i=0; i<sh_num; i++) {
        if (elf32_shdr[i].sh_type == SHT_SYMTAB) {
            elf32_shdr_entry_symtab = &elf32_shdr[i];
        }
        if (elf32_shdr[i].sh_type == SHT_STRTAB && i != elf32_ehdr.e_shstrndx) {
            elf32_shdr_entry_strtab = &elf32_shdr[i];
        }
    }
    assert (elf32_shdr_entry_symtab);
    assert (elf32_shdr_entry_strtab);

    // get the number of symbol table
    assert (elf32_shdr_entry_symtab->sh_size > 0 || elf32_shdr_entry_symtab->sh_entsize > 0);
    int symtab_num = elf32_shdr_entry_symtab->sh_size / elf32_shdr_entry_symtab->sh_entsize;
    assert (symtab_num > 0);
    printf("Number of symbol table: %d\n", symtab_num);

    // read symbol table
    fseek(elf_fp, elf32_shdr_entry_symtab->sh_offset, SEEK_SET);
    Elf32_Sym *elf32_symtab = malloc(sizeof(Elf32_Sym) * symtab_num);
    assert(elf32_symtab);
    assert(fread(elf32_symtab, sizeof(Elf32_Sym), symtab_num, elf_fp) == symtab_num);

    // get the size of string table
    assert(elf32_shdr_entry_strtab->sh_size > 0);
    int strtab_size = elf32_shdr_entry_strtab->sh_size;
    printf("The size of string table: %d\n", strtab_size);

    // read string table
    fseek(elf_fp, elf32_shdr_entry_strtab->sh_offset, SEEK_SET);
    char *elf32_strtab = malloc(sizeof(char) * strtab_size);
    assert(elf32_strtab);
    assert(fread(elf32_strtab, strtab_size, 1, elf_fp) == 1);

    // todo: print out symbol table
    printf("\nSymbol Table:\n");
    printf("   Num:    Value  Size Type Bind   Name\n");
    // 遍历符号表
    for (i = 0; i < symtab_num; i++) {
        printf("%6d: %08x %5d %4d %4d %s\n", 
                i, elf32_symtab[i].st_value, elf32_symtab[i].st_size, 
                ELF32_ST_TYPE(elf32_symtab[i].st_info), ELF32_ST_BIND(elf32_symtab[i].st_info),
                &elf32_strtab[elf32_symtab[i].st_name]);
    }
    
    
    
    
    // 跳转到符号表位置
    // fseek(elf_fp, elf32_shdr[i].sh_offset, SEEK_SET);
    // printf("\nSymbol Table:\n");
    // printf("   Num:    Value  Size Type Bind   Name\n");
    // // 遍历符号表
    // for (i = 0; i < symtab_num; i++) {
    //     Elf32_Sym symbol;
    //     if (fread(&symbol, sizeof(symbol), 1, elf_fp) != 1) {
    //         printf("Not a valid ELF symbol\n");
    //         fclose(elf_fp);
    //         assert(0);
    //     }

    //     // 解析符号类型和绑定属性
    //     // char *type;
    //     // switch (ELF32_ST_TYPE(symbol.st_info)) {
    //     //     case STT_NOTYPE: type = "NOTYPE"; break;
    //     //     case STT_OBJECT: type = "OBJECT"; break;
    //     //     case STT_FUNC:   type = "FUNC"; break;
    //     //     case STT_SECTION:type = "SECTION"; break;
    //     //     case STT_FILE:   type = "FILE"; break;
    //     //     default:         type = "UNKNOWN"; break;
    //     // }

    //     // const char *bind;
    //     // switch (ELF32_ST_BIND(symbol.st_info)) {
    //     //     case STB_LOCAL:  bind = "LOCAL"; break;
    //     //     case STB_GLOBAL: bind = "GLOBAL"; break;
    //     //     case STB_WEAK:   bind = "WEAK"; break;
    //     //     default:         bind = "UNKNOWN"; break;
    //     // }

    //     // 输出符号信息
    //     // printf("  %3d: %08x %5d %-7s %-6s %s\n",
    //     //        i, symbol.st_value, symbol.st_size, type, bind,
    //     //        &string_table[symbol.st_name]);
    //     printf("%6d: %08x %5d %4d %4d %d\n", 
    //             i, symbol.st_value, symbol.st_size, 
    //             ELF32_ST_TYPE(symbol.st_info), ELF32_ST_BIND(symbol.st_info),
    //             symbol.st_name);

    // }


    // Get symbol table '.symtab'
    // Elf32_Sym elf32_sym;
    // fread(&elf32_sym, sizeof(char), sizeof(elf32_sym), elf_fp);

    free(elf32_shdr);
    // free(elf32_shdr_entry_symtab);
    // free(elf32_shdr_entry_strtab);
    free(elf32_symtab);
    free(elf32_strtab);
    
    fclose(elf_fp);

}
