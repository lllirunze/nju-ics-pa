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
#include <memory/paddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
              TK_EQ, TK_NEQ, 
              TK_DEC, TK_HEX, TK_REG, 
              TK_AND, TK_OR,
              TK_SHIFTLEFT, TK_SHIFTRIGHT,
              TK_G, TK_GEQ, TK_L, TK_LEQ,
              TK_NEG, TK_DEREF

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
  int priority;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE, 0},             // spaces
  {"\\+", '+', 4},                  // plus
  {"-", '-', 4},                    // minus OR negation
  {"\\*", '*', 3},                  // multiplication OR dereference
  {"\\/", '/', 3},                  // division
  {"\\%", '%', 3},                  // mod
  {"\\(", '(', 1},                  // left parenthesis
  {"\\)", ')', 1},                  // right parenthesis
  {"0[xX][0-9a-fA-F]+", TK_HEX, 0}, // hexadecimal number -> 0x..(0)
  {"[0-9]+", TK_DEC, 0},            // decimal number
  {"\\$(0|ra|sp|gp|tp|t[0-6]|s10|s11|s[0-9]|a[0-7]|pc)", TK_REG, 0}, // registers
  {"==", TK_EQ, 7},                 // equal
  {"!=", TK_NEQ, 7},                // not equal
  {"&&", TK_AND, 11},               // and
  {"\\|\\|", TK_OR, 12},            // or
  {"!", '!', 2},                    // not
  {"&", '&', 8},                    // bitwise and (we don't consider taking address)
  {"\\|", '|', 10},                 // bitwise or
  {"\\^", '^', 9},                  // bitwise xor
  {"~", '~', 2},                    // bitwise inversion
  {"<<", TK_SHIFTLEFT, 5},          // shift left
  {">>", TK_SHIFTRIGHT, 5},         // shift right
  {">=", TK_GEQ, 6},                // greater than or equal to
  {"<=", TK_LEQ, 6},                // less than or equal to
  {">", TK_G, 6},                   // greater than
  {"<", TK_L, 6},                   // less than

};

#define NR_REGEX ARRLEN(rules)
#define NR_TOKENS 32

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
  int priority;
} Token;

static Token tokens[NR_TOKENS] __attribute__((used)) = {};
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
        position += substr_len;

        if (substr_len > 32) {
          printf("Error: The token is too long.\n");
          return false;
        }

        if (nr_token >= NR_TOKENS) {
          printf("Error: There is too many tokens.\n");
          return false;
        }

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case TK_EQ:
          case TK_NEQ:
          case '+':
          case '/':
          case '%':
          case '(':
          case ')':
          case TK_AND:
          case TK_OR:
          case '!':
          case '&':
          case '|':
          case '^':
          case '~':
          case TK_SHIFTLEFT:
          case TK_SHIFTRIGHT:
          case TK_GEQ:
          case TK_LEQ:
          case TK_G:
          case TK_L:
          case TK_HEX:
          case TK_DEC:
          case TK_REG:
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].priority = rules[i].priority;
            nr_token++;
            break;
          case '-':
            if (nr_token != 0 && (tokens[nr_token-1].type == TK_DEC || tokens[nr_token-1].type == TK_HEX || tokens[nr_token-1].type == TK_REG || tokens[nr_token-1].type == ')')) {
              tokens[nr_token].type = '-';
              tokens[nr_token].priority = 4;
            }
            else {
              tokens[nr_token].type = TK_NEG;
              tokens[nr_token].priority = 2;
            }
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            nr_token++;
            break;
          case '*':
            if (nr_token != 0 && (tokens[nr_token-1].type == TK_DEC || tokens[nr_token-1].type == TK_HEX || tokens[nr_token-1].type == TK_REG || tokens[nr_token-1].type == ')')) {
              tokens[nr_token].type = '*';
              tokens[nr_token].priority = 3;
            }
            else {
              tokens[nr_token].type = TK_DEREF;
              tokens[nr_token].priority = 2;
            }
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            nr_token++;
            break;
          default: break;
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

bool check_parentheses(int left, int right) {
  if (tokens[left].type != '(' || tokens[right].type != ')') return false;
  int layer = 0;
  int i;
  for (i = left; i < right; i++) {
    if (tokens[i].type == '(') layer++;
    else if (tokens[i].type == ')') layer--;
    if (layer <= 0) return false;
  }
  return (layer == 1);
}

int check_priority(int left, int right) {
  int dominate_priority = 0;
  int i;
  int layer = 0;
  for (i = left; i <= right; i++) {
    if (tokens[i].type == '(') layer++;
    else if (tokens[i].type == ')') layer--;
    if (layer != 0) continue;
    if (tokens[i].priority > dominate_priority) dominate_priority = tokens[i].priority;
  }
  return dominate_priority;
}

int find_dominate_operator(int left, int right, int pri, bool leftToRight) {
  int i;
  int layer = 0;
  if (leftToRight == false) {
    for (i = left; i <= right; i++) {
      if (tokens[i].type == '(') layer++;
      else if (tokens[i].type == ')') layer--;
      if (layer != 0) continue;
      if (tokens[i].priority == pri) return i;
    }
  }
  else {
    for (i = right; i >= left; i--) {
      if (tokens[i].type == '(') layer++;
      else if (tokens[i].type == ')') layer--;
      if (layer != 0) continue;
      if (tokens[i].priority == pri) return i;
    }
  }
  return 0;
}

word_t eval(int left, int right, bool *success) {
  if (left > right) {
    Log("Bad expression between %d and %d.", left, right);
    *success = false;
    return 0;
  }
  else if (left == right) {
    int num_type = tokens[left].type;
    switch(num_type) {
      case TK_HEX:
        char *hex_str = tokens[left].str+2;
        if (strlen(hex_str) <= 0 || strlen(hex_str) > 8) {
          Log("Hexadecimal number %s out of bounds.", hex_str);
          *success = false;
          return 0;
        }
        return (word_t)strtoul(hex_str, NULL, 16);
      case TK_DEC: 
        char *dec_str = tokens[left].str;
        return (word_t)strtoul(dec_str, NULL, 10);
      case TK_REG:
        char *reg_str;
        if (strcmp(tokens[left].str, "$0") == 0) reg_str = tokens[left].str;
        else reg_str = tokens[left].str+1;
        return isa_reg_str2val(reg_str, success);
      default:
        Log("Unknown number %s in the position %d.", tokens[left].str, left);
        *success = false;
        return 0;
    }
  }
  else if (check_parentheses(left, right) == true) {
    return eval(left+1, right-1, success);
  }
  else {
    int dominate_priority = check_priority(left, right);
    bool leftToRight;
    if (dominate_priority == 2 || dominate_priority == 13 || dominate_priority == 14) leftToRight = false;
    else leftToRight = true;

    int op = find_dominate_operator(left, right, dominate_priority, leftToRight);
    int op_type = tokens[op].type;

    word_t val1, val2;
    
    switch(op_type) {
      case '+': return eval(left, op-1, success) + eval(op+1, right, success);
      case '-': return eval(left, op-1, success) - eval(op+1, right, success);
      case '*': return eval(left, op-1, success) * eval(op+1, right, success);
      case '/':
        val1 = eval(left, op-1, success);
        val2 = eval(op+1, right, success);
        if (val2 == 0) {
          Log("Warning: The divisor cannot be 0.");
          *success = false;
          return 0;
        }
        return val1 / val2;
      case '%':
        val1 = eval(left, op-1, success);
        val2 = eval(op+1, right, success);
        if (val2 == 0) {
          Log("Warning: The divisor cannot be 0.");
          *success = false;
          return 0;
        }
        return val1 % val2;
      case TK_EQ: return eval(left, op-1, success) == eval(op+1, right, success);
      case TK_NEQ: return eval(left, op-1, success) != eval(op+1, right, success);
      case TK_AND: return eval(left, op-1, success) && eval(op+1, right, success);
      case TK_OR: return eval(left, op-1, success) || eval(op+1, right, success);
      case '!': return !eval(op+1, right, success);
      case '&': return eval(left, op-1, success) & eval(op+1, right, success);
      case '|': return eval(left, op-1, success) | eval(op+1, right, success);
      case '^': return eval(left, op-1, success) ^ eval(op+1, right, success);
      case '~': return ~eval(op+1, right, success);
      case TK_SHIFTLEFT: return eval(left, op-1, success) << eval(op+1, right, success);
      case TK_SHIFTRIGHT: return eval(left, op-1, success) >> eval(op+1, right, success);
      case TK_GEQ: return eval(left, op-1, success) >= eval(op+1, right, success);
      case TK_LEQ: return eval(left, op-1, success) <= eval(op+1, right, success);
      case TK_G: return eval(left, op-1, success) > eval(op+1, right, success);
      case TK_L: return eval(left, op-1, success) < eval(op+1, right, success);
      case TK_NEG: return -eval(op+1, right, success);
      case TK_DEREF:
        val2 = eval(op+1, right, success);
        if (!in_pmem(val2)) {
          Log("Warning: Address 0x%08x is out of bound of pmem [0x%08x, 0x%08x]", val2, PMEM_LEFT, PMEM_RIGHT);
          *success = false;
          return 0;
        }
        return vaddr_read(val2, 4);
      default:
        Log("Unknown operator %s in the position %d.", tokens[op].str, op);
        *success = false;
        return 0;
    }
  }
  return 0;
}

void free_token() {
  int i;
  for (i=0; i<NR_TOKENS; i++) {
    memset(tokens[i].str, 0, sizeof(tokens[i].str));
  }
  return;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  word_t result = eval(0, nr_token-1, success);
  free_token();
  return result;
}
