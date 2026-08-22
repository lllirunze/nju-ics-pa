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

typedef union {
  struct {
    uint8_t R_M		:3;
    uint8_t reg		:3;
    uint8_t mod		:2;
  };
  struct {
    uint8_t dont_care	:3;
    uint8_t opcode		:3;
  };
  uint8_t val;
} ModR_M;

typedef union {
  struct {
    uint8_t base	:3;
    uint8_t index	:3;
    uint8_t ss		:2;
  };
  uint8_t val;
} SIB;

static word_t x86_inst_fetch(Decode *s, int len) {
#if defined(CONFIG_ITRACE) || defined(CONFIG_IQUEUE)
  uint8_t *p = &s->isa.inst[s->snpc - s->pc];
  word_t ret = inst_fetch(&s->snpc, len);
  word_t ret_save = ret;
  int i;
  assert(s->snpc - s->pc < sizeof(s->isa.inst));
  for (i = 0; i < len; i ++) {
    p[i] = ret & 0xff;
    ret >>= 8;
  }
  return ret_save;
#else
  return inst_fetch(&s->snpc, len);
#endif
}

word_t reg_read(int idx, int width) {
  switch (width) {
    case 4: return reg_l(idx);
    case 1: return reg_b(idx);
    case 2: return reg_w(idx);
    default: assert(0);
  }
}

static void reg_write(int idx, int width, word_t data) {
  switch (width) {
    case 4: reg_l(idx) = data; return;
    case 1: reg_b(idx) = data; return;
    case 2: reg_w(idx) = data; return;
    default: assert(0);
  }
}

static void load_addr(Decode *s, ModR_M *m, word_t *rm_addr) {
  assert(m->mod != 3);

  sword_t disp = 0;
  int disp_size = 4;
  int base_reg = -1, index_reg = -1, scale = 0;

  if (m->R_M == R_ESP) {
    SIB sib;
    sib.val = x86_inst_fetch(s, 1);
    base_reg = sib.base;
    scale = sib.ss;

    if (sib.index != R_ESP) { index_reg = sib.index; }
  }
  else { base_reg = m->R_M; } /* no SIB */

  if (m->mod == 0) {
    if (base_reg == R_EBP) { base_reg = -1; }
    else { disp_size = 0; }
  }
  else if (m->mod == 1) { disp_size = 1; }

  if (disp_size != 0) { /* has disp */
    disp = x86_inst_fetch(s, disp_size);
    if (disp_size == 1) { disp = (int8_t)disp; }
  }

  word_t addr = disp;
  if (base_reg != -1)  addr += reg_l(base_reg);
  if (index_reg != -1) addr += reg_l(index_reg) << scale;
  *rm_addr = addr;
}

static void decode_rm(Decode *s, int *rm_reg, word_t *rm_addr, int *reg, int width) {
  ModR_M m;
  m.val = x86_inst_fetch(s, 1);
  if (reg != NULL) *reg = m.reg;
  if (m.mod == 3) *rm_reg = m.R_M;
  else { load_addr(s, &m, rm_addr); *rm_reg = -1; }
}

#define Rr reg_read
#define Rw reg_write
#define Mr vaddr_read
#define Mw vaddr_write
#define RMr(reg, w)  (reg != -1 ? Rr(reg, w) : Mr(addr, w))
#define RMw(data) do { if (rd != -1) Rw(rd, w, data); else Mw(addr, w, data); } while (0)

#define EFLAGS_CF 0x001
#define EFLAGS_ZF 0x040
#define EFLAGS_SF 0x080
#define EFLAGS_OF 0x800

static inline word_t width_mask(int w) {
  return w == 4 ? 0xffffffffu : (1u << (w * 8)) - 1;
}

static void set_zs(word_t result, int w) {
  word_t value = result & width_mask(w);
  cpu.eflags = (cpu.eflags & ~(EFLAGS_ZF | EFLAGS_SF)) |
      (value == 0 ? EFLAGS_ZF : 0) |
      (value & ((word_t)1 << (w * 8 - 1)) ? EFLAGS_SF : 0);
}

static void set_add_flags(word_t lhs, word_t rhs, word_t result, int w) {
  word_t mask = width_mask(w);
  word_t sign = (word_t)1 << (w * 8 - 1);
  lhs &= mask; rhs &= mask; result &= mask;
  cpu.eflags = (cpu.eflags & ~(EFLAGS_CF | EFLAGS_OF)) |
      ((uint64_t)lhs + rhs > mask ? EFLAGS_CF : 0) |
      ((~(lhs ^ rhs) & (lhs ^ result) & sign) ? EFLAGS_OF : 0);
  set_zs(result, w);
}

static void set_sub_flags(word_t lhs, word_t rhs, word_t result, int w) {
  word_t sign = (word_t)1 << (w * 8 - 1);
  cpu.eflags = (cpu.eflags & ~(EFLAGS_CF | EFLAGS_OF)) |
      (lhs < rhs ? EFLAGS_CF : 0) |
      (((lhs ^ rhs) & (lhs ^ result) & sign) ? EFLAGS_OF : 0);
  set_zs(result, w);
}

static bool condition(uint8_t cc) {
  bool cf = cpu.eflags & EFLAGS_CF;
  bool zf = cpu.eflags & EFLAGS_ZF;
  bool sf = cpu.eflags & EFLAGS_SF;
  bool of = cpu.eflags & EFLAGS_OF;
  switch (cc & 0xf) {
    case 0x0: return of;             case 0x1: return !of;
    case 0x2: return cf;             case 0x3: return !cf;
    case 0x4: return zf;             case 0x5: return !zf;
    case 0x6: return cf || zf;       case 0x7: return !cf && !zf;
    case 0x8: return sf;             case 0x9: return !sf;
    case 0xa: return false;          case 0xb: return true;
    case 0xc: return sf != of;       case 0xd: return sf == of;
    case 0xe: return zf || sf != of; case 0xf: return !zf && sf == of;
  }
  return false;
}

static void push(word_t value, int w) {
  cpu.esp -= w;
  vaddr_write(cpu.esp, w, value);
}

static word_t pop(int w) {
  word_t value = vaddr_read(cpu.esp, w);
  cpu.esp += w;
  return value;
}

static word_t x86_shift(word_t value, int w, int op, unsigned count) {
  word_t mask = width_mask(w);
  unsigned bits = w * 8;
  count &= 0x1f;
  value &= mask;
  if (count == 0) return value;

  word_t result;
  switch (op) {
    case 0: result = ((value << count) | (value >> (bits - count))) & mask; break; // rol
    case 1: result = ((value >> count) | (value << (bits - count))) & mask; break; // ror
    case 4: result = (value << count) & mask; break; // shl
    case 5: result = value >> count; break;          // shr
    case 7: result = (word_t)((sword_t)(value << (32 - bits)) >> (32 - bits)) >> count; break; // sar
    default: panic("unsupported x86 shift operation %d", op);
  }
  if (op >= 4) set_zs(result, w);
  if (op == 4) cpu.eflags = (cpu.eflags & ~EFLAGS_CF) | ((value >> (bits - count)) & 1 ? EFLAGS_CF : 0);
  if (op == 5 || op == 7) cpu.eflags = (cpu.eflags & ~EFLAGS_CF) | ((value >> (count - 1)) & 1 ? EFLAGS_CF : 0);
  return result;
}

static word_t cr_read(int index) {
  switch (index) {
    case 0: return cpu.cr0;
    case 2: return cpu.cr2;
    case 3: return cpu.cr3;
    default: panic("unsupported x86 control register cr%d", index);
  }
}

static void cr_write(int index, word_t value) {
  switch (index) {
    case 0: cpu.cr0 = value; return;
    case 2: cpu.cr2 = value; return;
    case 3: cpu.cr3 = value & 0xfffff000u; return;
    default: panic("unsupported x86 control register cr%d", index);
  }
}

#define destr(r)  do { *rd_ = (r); } while (0)
#define src1r(r)  do { *src1 = Rr(r, w); } while (0)
#define imm()     do { *imm = x86_inst_fetch(s, w); } while (0)
#define simm(w)   do { *imm = SEXT(x86_inst_fetch(s, w), w * 8); } while (0)

enum {
  TYPE_r, TYPE_I, TYPE_SI, TYPE_J, TYPE_E,
  TYPE_I2r,  // XX <- Ib / eXX <- Iv
  TYPE_I2a,  // AL <- Ib / eAX <- Iv
  TYPE_G2E,  // Eb <- Gb / Ev <- Gv
  TYPE_E2G,  // Gb <- Eb / Gv <- Ev
  TYPE_I2E,  // Eb <- Ib / Ev <- Iv
  TYPE_Ib2E, TYPE_cl2E, TYPE_1_E, TYPE_SI2E,
  TYPE_Eb2G, TYPE_Ew2G,
  TYPE_O2a, TYPE_a2O,
  TYPE_I_E2G,  // Gv <- EvIb / Gv <- EvIv // use for imul
  TYPE_SI_E2G,  // Gv <- EvIb / Gv <- EvIv // use for imul
  TYPE_Ib_G2E, // Ev <- GvIb // use for shld/shrd
  TYPE_cl_G2E, // Ev <- GvCL // use for shld/shrd
  TYPE_N, // none
};

#define INSTPAT_INST(s) opcode
#define INSTPAT_MATCH(s, name, type, width, ... /* execute body */ ) { \
  int rd = 0, rs = 0, gp_idx = 0; \
  word_t src1 = 0, addr = 0, imm = 0; \
  int w = width == 0 ? (is_operand_size_16 ? 2 : 4) : width; \
  decode_operand(s, opcode, &rd, &src1, &addr, &rs, &gp_idx, &imm, w, concat(TYPE_, type)); \
  /* Operand decoding may consume displacement and immediate bytes. */ \
  s->dnpc = s->snpc; \
  __VA_ARGS__ ; \
}

static void decode_operand(Decode *s, uint8_t opcode, int *rd_, word_t *src1,
    word_t *addr, int *rs, int *gp_idx, word_t *imm, int w, int type) {
  switch (type) {
    case TYPE_r:    destr(opcode & 0x7); break;
    case TYPE_I2r:  destr(opcode & 0x7); imm(); break;
    case TYPE_G2E:  decode_rm(s, rd_, addr, rs, w); src1r(*rs); break;
    case TYPE_E2G:  decode_rm(s, rs, addr, rd_, w); break;
    case TYPE_I2E:  decode_rm(s, rd_, addr, gp_idx, w); imm(); break;
    case TYPE_SI2E: decode_rm(s, rd_, addr, gp_idx, w); simm(1); break;
    case TYPE_O2a:  destr(R_EAX); *addr = x86_inst_fetch(s, 4); break;
    case TYPE_a2O:  *rs = R_EAX;  *addr = x86_inst_fetch(s, 4); break;
    case TYPE_N:    break;
    default: panic("Unsupported type = %d", type);
  }
}

#define gp1() do { \
  word_t lhs = RMr(rd, w); \
  word_t result = 0; \
  switch (gp_idx) { \
    case 0: result = lhs + imm; RMw(result); set_add_flags(lhs, imm, result, w); break; \
    case 1: result = lhs | imm; RMw(result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF); break; \
    case 2: { word_t carry = cpu.eflags & EFLAGS_CF ? 1 : 0; result = lhs + imm + carry; RMw(result); set_add_flags(lhs, imm + carry, result, w); break; } \
    case 3: { word_t carry = cpu.eflags & EFLAGS_CF ? 1 : 0; result = lhs - imm - carry; RMw(result); set_sub_flags(lhs, imm + carry, result, w); break; } \
    case 4: result = lhs & imm; RMw(result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF); break; \
    case 5: result = lhs - imm; RMw(result); set_sub_flags(lhs, imm, result, w); break; \
    case 6: result = lhs ^ imm; RMw(result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF); break; \
    case 7: result = lhs - imm; set_sub_flags(lhs, imm, result, w); break; \
    default: INV(s->pc); \
  }; \
} while (0)

void _2byte_esc(Decode *s, bool is_operand_size_16) {
  uint8_t opcode = x86_inst_fetch(s, 1);
  INSTPAT_START();
  INSTPAT("0000 0001", lidt,   N,    0, int rm, reg; decode_rm(s, &rm, &addr, &reg, 2); s->dnpc = s->snpc; if (reg != 3 || rm != -1) INV(s->pc); cpu.idtr.limit = Mr(addr, 2); cpu.idtr.base = Mr(addr + 2, 4));
  INSTPAT("0010 0000", mov_cr, N,    0, int rm, reg; decode_rm(s, &rm, &addr, &reg, 4); if (rm == -1) INV(s->pc); Rw(rm, 4, cr_read(reg)));
  INSTPAT("0010 0010", mov_cr, N,    0, int rm, reg; decode_rm(s, &rm, &addr, &reg, 4); if (rm == -1) INV(s->pc); cr_write(reg, Rr(rm, 4)));
  INSTPAT("1000 ????", jcc,    N,    0, word_t raw = x86_inst_fetch(s, is_operand_size_16 ? 2 : 4); sword_t off = is_operand_size_16 ? (int16_t)raw : (sword_t)raw; s->dnpc = condition(opcode & 0xf) ? s->snpc + off : s->snpc);
  INSTPAT("1001 ????", setcc,  N,    0, int rm, reg; word_t setcc_addr = 0; decode_rm(s, &rm, &setcc_addr, &reg, 1); s->dnpc = s->snpc; if (reg != 0) INV(s->pc); if (rm != -1) Rw(rm, 1, condition(opcode & 0xf)); else Mw(setcc_addr, 1, condition(opcode & 0xf)));
  INSTPAT("1011 0110", movzx,  E2G,  1, Rw(rd, is_operand_size_16 ? 2 : 4, RMr(rs, 1)));
  INSTPAT("1011 0111", movzx,  E2G,  2, Rw(rd, 4, RMr(rs, 2)));
  INSTPAT("1011 1110", movsx,  E2G,  1, Rw(rd, is_operand_size_16 ? 2 : 4, SEXT(RMr(rs, 1), 8)));
  INSTPAT("1011 1111", movsx,  E2G,  2, Rw(rd, 4, SEXT(RMr(rs, 2), 16)));
  INSTPAT("1010 1111", imul,   E2G,  0, Rw(rd, w, RMr(rs, w) * Rr(rd, w)));
  INSTPAT("???? ????", inv,    N,    0, INV(s->pc));
  INSTPAT_END();
}

int isa_exec_once(Decode *s) {
  bool is_operand_size_16 = false;
  uint8_t opcode = 0;

again:
  opcode = x86_inst_fetch(s, 1);

  INSTPAT_START();

  INSTPAT("0000 1111", 2byte_esc, N,    0, _2byte_esc(s, is_operand_size_16));

  INSTPAT("0110 0110", data_size, N,    0, is_operand_size_16 = true; goto again;);

  INSTPAT("1000 0000", gp1,       I2E,  1, gp1());
  INSTPAT("1000 0001", gp1,       I2E,  0, gp1());
  INSTPAT("1000 0011", gp1,       SI2E, 0, gp1());
  INSTPAT("0000 0001", add,       G2E,  0, word_t lhs = RMr(rd, w), result = lhs + src1; RMw(result); set_add_flags(lhs, src1, result, w));
  INSTPAT("0000 0011", add,       E2G,  0, word_t lhs = Rr(rd, w), rhs = RMr(rs, w), result = lhs + rhs; Rw(rd, w, result); set_add_flags(lhs, rhs, result, w));
  INSTPAT("0000 0101", add,       I2r,  0, word_t lhs = Rr(rd, w), result = lhs + imm; Rw(rd, w, result); set_add_flags(lhs, imm, result, w));
  INSTPAT("0000 0000", add,       G2E,  1, word_t lhs = RMr(rd, w), result = lhs + src1; RMw(result); set_add_flags(lhs, src1, result, w));
  INSTPAT("0000 0010", add,       E2G,  1, word_t lhs = Rr(rd, w), rhs = RMr(rs, w), result = lhs + rhs; Rw(rd, w, result); set_add_flags(lhs, rhs, result, w));
  INSTPAT("0001 0011", adc,       E2G,  0, word_t lhs = Rr(rd, w), rhs = RMr(rs, w) + (cpu.eflags & EFLAGS_CF ? 1 : 0), result = lhs + rhs; Rw(rd, w, result); set_add_flags(lhs, rhs, result, w));
  INSTPAT("0001 0101", adc,       I2r,  0, word_t rhs = imm + (cpu.eflags & EFLAGS_CF ? 1 : 0), lhs = Rr(rd, w), result = lhs + rhs; Rw(rd, w, result); set_add_flags(lhs, rhs, result, w));
  INSTPAT("0001 1011", sbb,       E2G,  0, word_t lhs = Rr(rd, w), rhs = RMr(rs, w) + (cpu.eflags & EFLAGS_CF ? 1 : 0), result = lhs - rhs; Rw(rd, w, result); set_sub_flags(lhs, rhs, result, w));
  INSTPAT("0001 1001", sbb,       G2E,  0, word_t lhs = RMr(rd, w), rhs = src1 + (cpu.eflags & EFLAGS_CF ? 1 : 0), result = lhs - rhs; RMw(result); set_sub_flags(lhs, rhs, result, w));
  INSTPAT("0010 1001", sub,       G2E,  0, word_t lhs = RMr(rd, w), result = lhs - src1; RMw(result); set_sub_flags(lhs, src1, result, w));
  INSTPAT("0010 1011", sub,       E2G,  0, word_t lhs = Rr(rd, w), rhs = RMr(rs, w), result = lhs - rhs; Rw(rd, w, result); set_sub_flags(lhs, rhs, result, w));
  INSTPAT("0010 1101", sub,       I2r,  0, word_t lhs = Rr(rd, w), result = lhs - imm; Rw(rd, w, result); set_sub_flags(lhs, imm, result, w));
  INSTPAT("0011 1001", cmp,       G2E,  0, word_t lhs = RMr(rd, w); set_sub_flags(lhs, src1, lhs - src1, w));
  INSTPAT("0011 1011", cmp,       E2G,  0, word_t lhs = Rr(rd, w), rhs = RMr(rs, w); set_sub_flags(lhs, rhs, lhs - rhs, w));
  INSTPAT("0011 1000", cmp,       G2E,  1, word_t lhs = RMr(rd, w); set_sub_flags(lhs, src1, lhs - src1, w));
  INSTPAT("0011 1010", cmp,       E2G,  1, word_t lhs = Rr(rd, w), rhs = RMr(rs, w); set_sub_flags(lhs, rhs, lhs - rhs, w));
  INSTPAT("0011 1100", cmp,       I2r,  1, word_t lhs = Rr(rd, w); set_sub_flags(lhs, imm, lhs - imm, w));
  INSTPAT("0011 1101", cmp,       I2r,  0, word_t lhs = Rr(rd, w); set_sub_flags(lhs, imm, lhs - imm, w));
  INSTPAT("0010 0001", and,       G2E,  0, word_t result = RMr(rd, w) & src1; RMw(result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("0010 0011", and,       E2G,  0, word_t result = Rr(rd, w) & RMr(rs, w); Rw(rd, w, result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("0010 0010", and,       E2G,  1, word_t result = Rr(rd, w) & RMr(rs, w); Rw(rd, w, result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("0010 0101", and,       I2r,  0, word_t result = Rr(rd, w) & imm; Rw(rd, w, result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("0000 1001", or,        G2E,  0, word_t result = RMr(rd, w) | src1; RMw(result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("0000 1011", or,        E2G,  0, word_t result = Rr(rd, w) | RMr(rs, w); Rw(rd, w, result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("0000 1010", or,        E2G,  1, word_t result = Rr(rd, w) | RMr(rs, w); Rw(rd, w, result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("0011 0001", xor,       G2E,  0, word_t result = RMr(rd, w) ^ src1; RMw(result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("0011 0011", xor,       E2G,  0, word_t result = Rr(rd, w) ^ RMr(rs, w); Rw(rd, w, result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("0011 0010", xor,       E2G,  1, word_t result = Rr(rd, w) ^ RMr(rs, w); Rw(rd, w, result); set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("1000 0101", test,      G2E,  0, word_t result = RMr(rd, w) & src1; set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("1000 0100", test,      G2E,  1, word_t result = RMr(rd, w) & src1; set_zs(result, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF));
  INSTPAT("1000 1000", mov,       G2E,  1, RMw(src1));
  INSTPAT("1000 1001", mov,       G2E,  0, RMw(src1));
  INSTPAT("1000 1010", mov,       E2G,  1, Rw(rd, w, RMr(rs, w)));
  INSTPAT("1000 1011", mov,       E2G,  0, Rw(rd, w, RMr(rs, w)));
  INSTPAT("1000 1101", lea,       E2G,  0, if (rs == -1) Rw(rd, w, addr); else INV(s->pc));

  INSTPAT("1010 0000", mov,       O2a,  1, Rw(R_EAX, 1, Mr(addr, 1)));
  INSTPAT("1010 0001", mov,       O2a,  0, Rw(R_EAX, w, Mr(addr, w)));
  INSTPAT("1010 0010", mov,       a2O,  1, Mw(addr, 1, Rr(R_EAX, 1)));
  INSTPAT("1010 0011", mov,       a2O,  0, Mw(addr, w, Rr(R_EAX, w)));

  INSTPAT("1011 0???", mov,       I2r,  1, Rw(rd, 1, imm));
  INSTPAT("1011 1???", mov,       I2r,  0, Rw(rd, w, imm));

  INSTPAT("0100 0???", inc,       r,    0, word_t lhs = Rr(rd, w), result = lhs + 1; Rw(rd, w, result); set_add_flags(lhs, 1, result, w));
  INSTPAT("0100 1???", dec,       r,    0, word_t lhs = Rr(rd, w), result = lhs - 1; Rw(rd, w, result); set_sub_flags(lhs, 1, result, w));
  INSTPAT("0101 0???", push,      r,    0, push(Rr(rd, w), w));
  INSTPAT("0101 1???", pop,       r,    0, Rw(rd, w, pop(w)));
  INSTPAT("0110 1000", push,      N,    0, word_t value = x86_inst_fetch(s, is_operand_size_16 ? 2 : 4); s->dnpc = s->snpc; push(value, is_operand_size_16 ? 2 : 4));
  INSTPAT("0110 1010", push,      N,    0, word_t value = SEXT(x86_inst_fetch(s, 1), 8); s->dnpc = s->snpc; push(value, is_operand_size_16 ? 2 : 4));
  INSTPAT("0110 0000", pusha,     N,    0, word_t sp = cpu.esp; push(cpu.eax, 4); push(cpu.ecx, 4); push(cpu.edx, 4); push(cpu.ebx, 4); push(sp, 4); push(cpu.ebp, 4); push(cpu.esi, 4); push(cpu.edi, 4));
  INSTPAT("0110 0001", popa,      N,    0, cpu.edi = pop(4); cpu.esi = pop(4); cpu.ebp = pop(4); pop(4); cpu.ebx = pop(4); cpu.edx = pop(4); cpu.ecx = pop(4); cpu.eax = pop(4));
  INSTPAT("1001 0000", nop,       N,    0, );
  INSTPAT("1001 1100", pushf,     N,    0, push(cpu.eflags, is_operand_size_16 ? 2 : 4));
  INSTPAT("1001 1101", popf,      N,    0, cpu.eflags = pop(is_operand_size_16 ? 2 : 4) | 0x2);
  INSTPAT("1001 1001", cdq,       N,    0, cpu.edx = cpu.eax & 0x80000000u ? 0xffffffffu : 0);
  INSTPAT("1100 1001", leave,     N,    0, cpu.esp = cpu.ebp; cpu.ebp = pop(is_operand_size_16 ? 2 : 4));
  INSTPAT("1100 0011", ret,       N,    0, s->dnpc = pop(is_operand_size_16 ? 2 : 4));
  INSTPAT("1100 1101", int,       N,    0, word_t no = x86_inst_fetch(s, 1); s->dnpc = isa_raise_intr(no, s->snpc));
  INSTPAT("1100 1111", iret,      N,    0, s->dnpc = pop(4); pop(4); cpu.eflags = pop(4) | 0x2);
  INSTPAT("1110 1000", call,      N,    0, word_t raw = x86_inst_fetch(s, is_operand_size_16 ? 2 : 4); sword_t off = is_operand_size_16 ? (int16_t)raw : (sword_t)raw; push(s->snpc, is_operand_size_16 ? 2 : 4); s->dnpc = s->snpc + off);
  INSTPAT("1110 1001", jmp,       N,    0, word_t raw = x86_inst_fetch(s, is_operand_size_16 ? 2 : 4); sword_t off = is_operand_size_16 ? (int16_t)raw : (sword_t)raw; s->dnpc = s->snpc + off);
  INSTPAT("1110 1011", jmp,       N,    0, int8_t off = x86_inst_fetch(s, 1); s->dnpc = s->snpc + off);
  INSTPAT("0111 ????", jcc,       N,    0, int8_t off = x86_inst_fetch(s, 1); s->dnpc = condition(opcode & 0xf) ? s->snpc + off : s->snpc);
  INSTPAT("1111 1010", cli,       N,    0, cpu.eflags &= ~((word_t)1 << 9));
  INSTPAT("1111 1011", sti,       N,    0, cpu.eflags |= (word_t)1 << 9);
  INSTPAT("1100 0000", group2,    N,    0,
      int rm, reg; word_t shift_addr = 0; decode_rm(s, &rm, &shift_addr, &reg, 1); unsigned count = x86_inst_fetch(s, 1); s->dnpc = s->snpc;
      word_t value = rm != -1 ? Rr(rm, 1) : Mr(shift_addr, 1); value = x86_shift(value, 1, reg, count); if (rm != -1) Rw(rm, 1, value); else Mw(shift_addr, 1, value));
  INSTPAT("1100 0001", group2,    N,    0,
      int rm, reg; word_t shift_addr = 0; decode_rm(s, &rm, &shift_addr, &reg, w); unsigned count = x86_inst_fetch(s, 1); s->dnpc = s->snpc;
      word_t value = rm != -1 ? Rr(rm, w) : Mr(shift_addr, w); value = x86_shift(value, w, reg, count); if (rm != -1) Rw(rm, w, value); else Mw(shift_addr, w, value));
  INSTPAT("1101 0000", group2,    N,    0,
      int rm, reg; word_t shift_addr = 0; decode_rm(s, &rm, &shift_addr, &reg, 1); s->dnpc = s->snpc;
      word_t value = rm != -1 ? Rr(rm, 1) : Mr(shift_addr, 1); value = x86_shift(value, 1, reg, 1); if (rm != -1) Rw(rm, 1, value); else Mw(shift_addr, 1, value));
  INSTPAT("1101 0001", group2,    N,    0,
      int rm, reg; word_t shift_addr = 0; decode_rm(s, &rm, &shift_addr, &reg, w); s->dnpc = s->snpc;
      word_t value = rm != -1 ? Rr(rm, w) : Mr(shift_addr, w); value = x86_shift(value, w, reg, 1); if (rm != -1) Rw(rm, w, value); else Mw(shift_addr, w, value));
  INSTPAT("1101 0010", group2,    N,    0,
      int rm, reg; word_t shift_addr = 0; decode_rm(s, &rm, &shift_addr, &reg, 1); s->dnpc = s->snpc;
      word_t value = rm != -1 ? Rr(rm, 1) : Mr(shift_addr, 1); value = x86_shift(value, 1, reg, reg_b(R_ECX)); if (rm != -1) Rw(rm, 1, value); else Mw(shift_addr, 1, value));
  INSTPAT("1101 0011", group2,    N,    0,
      int rm, reg; word_t shift_addr = 0; decode_rm(s, &rm, &shift_addr, &reg, w); s->dnpc = s->snpc;
      word_t value = rm != -1 ? Rr(rm, w) : Mr(shift_addr, w); value = x86_shift(value, w, reg, reg_b(R_ECX)); if (rm != -1) Rw(rm, w, value); else Mw(shift_addr, w, value));
  INSTPAT("1111 0110", group3,    N,    0,
      int rm, reg; word_t group_addr = 0; decode_rm(s, &rm, &group_addr, &reg, 1); word_t value = rm != -1 ? Rr(rm, 1) : Mr(group_addr, 1);
      if (reg == 0) { word_t imm = x86_inst_fetch(s, 1); set_zs(value & imm, 1); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF); } else if (reg == 2) { value = ~value; if (rm != -1) Rw(rm, 1, value); else Mw(group_addr, 1, value); } else INV(s->pc); s->dnpc = s->snpc);
  INSTPAT("1111 0111", group3,    N,    0,
      int rm, reg; word_t group_addr = 0; decode_rm(s, &rm, &group_addr, &reg, w); word_t value = rm != -1 ? Rr(rm, w) : Mr(group_addr, w); s->dnpc = s->snpc;
      if (reg == 0) { word_t imm = x86_inst_fetch(s, w); s->dnpc = s->snpc; set_zs(value & imm, w); cpu.eflags &= ~(EFLAGS_CF | EFLAGS_OF); }
      else if (reg == 2) { value = ~value; if (rm != -1) Rw(rm, w, value); else Mw(group_addr, w, value); }
      else if (reg == 3) { word_t result = -value; if (rm != -1) Rw(rm, w, result); else Mw(group_addr, w, result); set_sub_flags(0, value, result, w); }
      else if (reg == 4) { uint64_t result = (uint64_t)cpu.eax * value; cpu.eax = result; cpu.edx = result >> 32; }
      else if (reg == 5) { int64_t result = (int64_t)(sword_t)cpu.eax * (sword_t)value; cpu.eax = result; cpu.edx = result >> 32; }
      else if (reg == 6 && value != 0) { uint64_t dividend = ((uint64_t)cpu.edx << 32) | cpu.eax; cpu.eax = dividend / value; cpu.edx = dividend % value; }
      else if (reg == 7 && value != 0) { int64_t dividend = ((int64_t)(sword_t)cpu.edx << 32) | cpu.eax; cpu.eax = dividend / (sword_t)value; cpu.edx = dividend % (sword_t)value; }
      else INV(s->pc));
  INSTPAT("1111 1111", group5,    N,    0,
      int rm, reg; word_t group_addr = 0; decode_rm(s, &rm, &group_addr, &reg, w); s->dnpc = s->snpc;
      word_t value = rm != -1 ? Rr(rm, w) : Mr(group_addr, w);
      switch (reg) {
        case 0: { word_t result = value + 1; if (rm != -1) Rw(rm, w, result); else Mw(group_addr, w, result); set_add_flags(value, 1, result, w); break; }
        case 1: { word_t result = value - 1; if (rm != -1) Rw(rm, w, result); else Mw(group_addr, w, result); set_sub_flags(value, 1, result, w); break; }
        case 2: push(s->snpc, w); s->dnpc = value; break;
        case 4: s->dnpc = value; break;
        case 6: push(value, w); break;
        default: INV(s->pc);
      });

  INSTPAT("1100 0110", mov,       I2E,  1, RMw(imm));
  INSTPAT("1100 0111", mov,       I2E,  0, RMw(imm));
  INSTPAT("1100 1100", nemu_trap, N,    0, NEMUTRAP(s->pc, cpu.eax));
  INSTPAT("???? ????", inv,       N,    0, INV(s->pc));
  INSTPAT_END();

  return 0;
}
