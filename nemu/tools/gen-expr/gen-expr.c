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

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static size_t buf_pos = 0;
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int choose(int n) {
  return rand() % n;
}

static void gen(const char *s) {
  size_t len = strlen(s);
  assert(buf_pos + len < sizeof(buf));
  memcpy(buf + buf_pos, s, len + 1);
  buf_pos += len;
}

static uint32_t gen_rand_expr_internal(int depth);

static uint32_t gen_num(bool nonzero) {
  uint32_t value = choose(100);
  if (nonzero && value == 0) value = 1;

  char num[16];
  snprintf(num, sizeof(num), "%u", value);
  gen(num);
  return value;
}

static uint32_t gen_rand_expr_internal(int depth) {
  enum { MAX_DEPTH = 4, MAX_VALUE = 1000000000 };

  if (depth == MAX_DEPTH || choose(3) == 0) {
    return gen_num(false);
  }

  if (choose(2) == 0) {
    gen("(");
    uint32_t value = gen_rand_expr_internal(depth + 1);
    gen(")");
    return value;
  }

  size_t start = buf_pos;
  for (int attempt = 0; attempt < 16; attempt ++) {
    buf_pos = start;
    buf[buf_pos] = '\0';

    gen("(");
    uint32_t lhs = gen_rand_expr_internal(depth + 1);
    char op = "+-*/"[choose(4)];
    gen((char []){op, '\0'});
    uint32_t rhs = gen_rand_expr_internal(depth + 1);
    uint64_t value;

    switch (op) {
      case '+': value = (uint64_t)lhs + rhs; break;
      case '-':
        if (lhs < rhs) continue;
        value = lhs - rhs;
        break;
      case '*': value = (uint64_t)lhs * rhs; break;
      case '/':
        if (rhs == 0) continue;
        value = lhs / rhs;
        break;
      default: assert(0);
    }

    if (value <= MAX_VALUE) {
      gen(")");
      return value;
    }
  }

  buf_pos = start;
  buf[buf_pos] = '\0';
  return gen_num(false);
}

static void gen_rand_expr() {
  buf_pos = 0;
  buf[0] = '\0';
  gen_rand_expr_internal(0);
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  char code_file[64];
  char expr_file[64];
  char command[160];
  snprintf(code_file, sizeof(code_file), "/tmp/.gen-expr-%ld.c", (long)getpid());
  snprintf(expr_file, sizeof(expr_file), "/tmp/.gen-expr-%ld", (long)getpid());

  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen(code_file, "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    snprintf(command, sizeof(command), "gcc %s -o %s", code_file, expr_file);
    int ret = system(command);
    if (ret != 0) continue;

    fp = popen(expr_file, "r");
    assert(fp != NULL);

    unsigned result;
    ret = fscanf(fp, "%u", &result);
    int status = pclose(fp);
    if (ret != 1 || status != 0) {
      remove(code_file);
      remove(expr_file);
      continue;
    }

    printf("%u %s\n", result, buf);
  }
  remove(code_file);
  remove(expr_file);
  return 0;
}
