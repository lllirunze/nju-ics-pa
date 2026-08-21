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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <errno.h>

enum {
  TK_NOTYPE = 256, TK_NUM, TK_REG, TK_EQ, TK_NEQ, TK_AND, TK_DEREF, TK_NEG,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  {" +", TK_NOTYPE},    // spaces
  {"0[xX][0-9a-fA-F]+", TK_NUM},
  {"[0-9]+", TK_NUM},
  {"\\$[a-zA-Z0-9]+", TK_REG},
  {"==", TK_EQ},
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"\\+", '+'},         // plus
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[1024] = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        if (rules[i].token_type != TK_NOTYPE) {
          if (nr_token == ARRLEN(tokens)) {
            printf("Expression is too long\n");
            return false;
          }
          if (substr_len >= sizeof(tokens[nr_token].str)) {
            printf("Token is too long\n");
            return false;
          }

          tokens[nr_token].type = rules[i].token_type;
          if (rules[i].token_type == TK_NUM || rules[i].token_type == TK_REG) {
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
          }
          nr_token ++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static int precedence(int type) {
  switch (type) {
    case TK_AND: return 1;
    case TK_EQ: case TK_NEQ: return 2;
    case '+': case '-': return 3;
    case '*': case '/': return 4;
    default: return 0;
  }
}

static bool check_parentheses(int p, int q) {
  if (tokens[p].type != '(' || tokens[q].type != ')') {
    return false;
  }

  int level = 0;
  for (int i = p; i <= q; i ++) {
    if (tokens[i].type == '(') level ++;
    if (tokens[i].type == ')') level --;
    if (level == 0 && i < q) return false;
    if (level < 0) return false;
  }
  return level == 0;
}

static int find_main_operator(int p, int q, bool *success) {
  int op = -1;
  int min_precedence = 5;
  int level = 0;

  for (int i = p; i <= q; i ++) {
    if (tokens[i].type == '(') {
      level ++;
      continue;
    }
    if (tokens[i].type == ')') {
      if (-- level < 0) {
        *success = false;
        return -1;
      }
      continue;
    }
    if (level == 0 && precedence(tokens[i].type) <= min_precedence &&
        precedence(tokens[i].type) != 0) {
      min_precedence = precedence(tokens[i].type);
      op = i;
    }
  }

  if (level != 0) *success = false;
  return op;
}

static word_t eval(int p, int q, bool *success) {
  if (p > q) {
    *success = false;
    return 0;
  }

  if (p == q) {
    if (tokens[p].type == TK_REG) {
      return isa_reg_str2val(tokens[p].str + 1, success);
    }

    if (tokens[p].type != TK_NUM) {
      *success = false;
      return 0;
    }

    char *end = NULL;
    errno = 0;
    unsigned long value = strtoul(tokens[p].str, &end, 0);
    if (errno != 0 || *end != '\0') {
      *success = false;
      return 0;
    }
    return (word_t)value;
  }

  if (check_parentheses(p, q)) {
    return eval(p + 1, q - 1, success);
  }

  int op = find_main_operator(p, q, success);
  if (!*success) {
    *success = false;
    return 0;
  }

  if (op == -1) {
    if (tokens[p].type != TK_DEREF && tokens[p].type != TK_NEG) {
      *success = false;
      return 0;
    }

    word_t operand = eval(p + 1, q, success);
    if (!*success) return 0;
    return tokens[p].type == TK_DEREF ? vaddr_read(operand, 4) : -operand;
  }

  word_t val1 = eval(p, op - 1, success);
  if (!*success) return 0;
  if (tokens[op].type == TK_AND && val1 == 0) return 0;
  word_t val2 = eval(op + 1, q, success);
  if (!*success) return 0;

  switch (tokens[op].type) {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/':
      if (val2 == 0) {
        *success = false;
        return 0;
      }
      return val1 / val2;
    case TK_EQ: return val1 == val2;
    case TK_NEQ: return val1 != val2;
    case TK_AND: return val1 && val2;
    default: *success = false; return 0;
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  if (nr_token == 0) {
    *success = false;
    return 0;
  }

  for (int i = 0; i < nr_token; i ++) {
    bool unary = i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_REG &&
                            tokens[i - 1].type != ')');
    if (unary && tokens[i].type == '*') {
      tokens[i].type = TK_DEREF;
    }
    if (unary && tokens[i].type == '-') {
      tokens[i].type = TK_NEG;
    }
  }

  *success = true;
  return eval(0, nr_token - 1, success);
}
