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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

// enum {
//   TYPE_I, TYPE_U, TYPE_S,
//   TYPE_N, // none
// };

enum {
  TYPE_R, TYPE_I, TYPE_S, TYPE_B, TYPE_U, TYPE_J,
  TYPE_N, // none
};

/**
 * |--------|--------------------------------------------------------------------------------------|
 * |   bit  |    31   |30       25|24   21|    20   |19   15|14    12|11       8|    7    |6      0|
 * |--------|---------------------------------------|-------|--------|--------------------|--------|
 * | R-type |       funct7        |       rs2       |  rs1  | funct3 |        rd          | opcode |
 * |--------|---------------------------------------|-------|--------|--------------------|--------|
 * | I-type |               imm[11:0]               |  rs1  | funct3 |        rd          | opcode |
 * |--------|---------------------------------------|-------|--------|--------------------|--------|
 * | S-type |      imm[11:5]      |       rs2       |  rs1  | funct3 |     imm[4:0]       | opcode |
 * |--------|---------------------------------------|-------|--------|--------------------|--------|
 * | B-type | imm[12] | imm[10:5] |       rs2       |  rs1  | funct3 | imm[4:1] | imm[11] | opcode |
 * |--------|--------------------------------------------------------|--------------------|--------|
 * | U-type |                    imm[31:12]                          |        rd          | opcode |
 * |--------|--------------------------------------------------------|--------------------|--------|
 * | J-type | imm[20] |     imm[10:1]     | imm[11] |   imm[19:12]   |        rd          | opcode |
 * |--------|--------------------------------------------------------------------------------------|
*/

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
// #define immR() do {} while(0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immB() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | (BITS(i, 7, 7) << 11) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immJ() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 20) | (BITS(i, 19, 12) << 12) | (BITS(i, 20, 20) << 11) | (BITS(i, 30, 21) << 1); } while(0)

#define xlen sizeof(word_t)*8

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_R: src1R(); src2R();         break;
    case TYPE_I: src1R();          immI(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_B: src1R(); src2R(); immB(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_J:                   immJ(); break;
    case TYPE_N: break;
    default: panic("unsupported type = %d", type);
  }
}

static int decode_exec(Decode *s) {
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  int rd = 0; \
  word_t src1 = 0, src2 = 0, imm = 0; \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  // execute
  INSTPAT_START();

  /**
   * INSTPAT(pattern string, instruction name, instruction type, execution operation); 
   * 
                INSTPAT("??????? ????? ????? ??? ????? ????? ??",        ,  , );
   */

  // todo: riscv-tests fail: div, divu, rem, remu

  // RV32I Base Instruction Set
  /* lui     */ INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm);
  /* auipc   */ INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
  /* jal     */ INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, R(rd) = s->snpc, s->dnpc = s->pc + imm);
  /* jalr    */ INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, R(rd) = s->snpc, s->dnpc = (src1 + imm) & (~1));
  /* beq     */ INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, ({if (src1 == src2) s->dnpc = s->pc + imm;}));
  /* bne     */ INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, ({if (src1 != src2) s->dnpc = s->pc + imm;}));
  /* blt     */ INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, ({if ((sword_t)src1 < (sword_t)src2) s->dnpc = s->pc + imm;}));
  /* bge     */ INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, ({if ((sword_t)src1 >= (sword_t)src2) s->dnpc = s->pc + imm;}));
  /* bltu    */ INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, ({if (src1 < src2) s->dnpc = s->pc + imm;}));
  /* bgeu    */ INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, ({if (src1 >= src2) s->dnpc = s->pc + imm;}));
  /* lb      */ INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(rd) = SEXT(BITS(Mr(src1 + imm, 1), 7, 0), 8));
  /* lh      */ INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(rd) = SEXT(BITS(Mr(src1 + imm, 2), 15, 0), 16));
  /* lw      */ INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = Mr(src1 + imm, 4));
  /* lbu     */ INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
  /* lhu     */ INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(rd) = Mr(src1 + imm, 2));
  /* sb      */ INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));
  /* sh      */ INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2));
  /* sw      */ INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  /* addi    */ INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(rd) = src1 + (sword_t)imm);
  /* slti    */ INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(rd) = ((sword_t)src1 < (sword_t)imm) ? 1 : 0);
  /* sltiu   */ INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(rd) = (src1 < imm) ? 1 : 0);
  /* xori    */ INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(rd) = src1 ^ imm);
  /* ori     */ INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(rd) = src1 | imm);
  /* andi    */ INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(rd) = src1 & imm);
  /* slli    */ INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , I, R(rd) = src1 << BITS(imm, 4, 0));
  /* srli    */ INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , I, R(rd) = src1 >> BITS(imm, 4, 0));
  /* srai    */ INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   , I, R(rd) = (sword_t)src1 >> BITS(imm, 4, 0));
  /* and     */ INSTPAT("0000000 ????? ????? 000 ????? 01100 11", and    , R, R(rd) = src1 + src2);
  /* sub     */ INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(rd) = src1 - src2);
  /* sll     */ INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(rd) = src1 << BITS(src2, 4, 0));
  /* slt     */ INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(rd) = ((sword_t)src1 < (sword_t)src2) ? 1 : 0);
  /* sltu    */ INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(rd) = (src1 < src2) ? 1 : 0);
  /* xor     */ INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(rd) = src1 ^ src2);
  /* srl     */ INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(rd) = src1 >> BITS(src2, 4, 0));
  /* sra     */ INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, R(rd) = (sword_t)src1 >> BITS(src2, 4, 0));
  /* or      */ INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(rd) = src1 | src2);
  /* and     */ INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(rd) = src1 & src2);
  /* fence   */
  /* ecall   */
  /* ebreak  */ INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0

  // RV32M Standard Extension
  /* mul     */ INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(rd) = (int32_t)src1 * (int32_t)src2);
  /* mulh    */ INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, R(rd) = (SEXT(src1, 32) * SEXT(src2, 32)) >> xlen);
  /* mulhsu  */ INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu , R, R(rd) = (SEXT(src1, 32) * (uint64_t)src2) >> xlen);
  /* mulhu   */ INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, R(rd) = ((uint64_t)src1 * (uint64_t)src2) >> xlen);
  /* div     */ INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, ({
                                                                                if ((int32_t)src2 == 0) {
                                                                                  R(rd) = (int32_t)(-1);
                                                                                }
                                                                                else if ((int32_t)src1 == (int32_t)0x80000000 && (int32_t)src2 == (int32_t)(-1)) {
                                                                                  R(rd) = 0x80000000;
                                                                                }
                                                                                else {
                                                                                  R(rd) = (int32_t)src1 / (int32_t)src2;
                                                                                }
                                                                              }));
  /* divu    */ INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, ({
                                                                                if (src2 == 0) {
                                                                                  R(rd) = 0xffffffff;
                                                                                } 
                                                                                else {
                                                                                  R(rd) = src1 / src2;
                                                                                }
                                                                              }));
  /* rem     */ INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, ({
                                                                                if ((int32_t)src2 == 0) {
                                                                                  R(rd) = (int32_t)src1;
                                                                                }
                                                                                else if ((int32_t)src1 == (int32_t)0x80000000 && (int32_t)src2 == (int32_t)(-1)) {
                                                                                  R(rd) = 0;
                                                                                }
                                                                                else {
                                                                                  R(rd) = (int32_t)src1 % (int32_t)src2;
                                                                                }
                                                                              }));
  /* remu    */ INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, ({
                                                                                if (src2 == 0) {
                                                                                  R(rd) = src1;
                                                                                } 
                                                                                else {
                                                                                  R(rd) = src1 % src2;
                                                                                }
                                                                              }));

  // If all the previous pattern matching rules fail to match successfully, the instruction is considered illegal.
  /* inv     */ INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  // instrcution fetch
  s->isa.inst = inst_fetch(&s->snpc, 4);
  // instruction decode
  return decode_exec(s);
}
