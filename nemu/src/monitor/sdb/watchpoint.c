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
#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expression[1024];
  word_t value;

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

static WP *new_wp() {
  assert(free_ != NULL);

  WP *wp = free_;
  free_ = free_->next;
  wp->next = head;
  head = wp;
  return wp;
}

static void free_wp(WP *wp) {
  WP **link = &head;
  while (*link != NULL && *link != wp) link = &(*link)->next;
  assert(*link == wp);

  *link = wp->next;
  wp->next = free_;
  free_ = wp;
}

bool add_watchpoint(const char *expression, word_t value, int *number) {
  if (strlen(expression) >= sizeof(wp_pool[0].expression)) return false;

  WP *wp = new_wp();
  strcpy(wp->expression, expression);
  wp->value = value;
  *number = wp->NO;
  return true;
}

bool delete_watchpoint(int number) {
  for (WP *wp = head; wp != NULL; wp = wp->next) {
    if (wp->NO == number) {
      free_wp(wp);
      return true;
    }
  }
  return false;
}

void display_watchpoints() {
  if (head == NULL) {
    printf("No watchpoints.\n");
    return;
  }

  printf("Num\tValue\tExpression\n");
  for (WP *wp = head; wp != NULL; wp = wp->next) {
    printf("%d\t" FMT_WORD "\t%s\n", wp->NO, wp->value, wp->expression);
  }
}

void check_watchpoints() {
  for (WP *wp = head; wp != NULL; wp = wp->next) {
    bool success = false;
    word_t value = expr(wp->expression, &success);
    if (!success) {
      printf("Watchpoint %d: cannot evaluate '%s'\n", wp->NO, wp->expression);
      nemu_state.state = NEMU_STOP;
      continue;
    }

    if (value != wp->value) {
      printf("Watchpoint %d triggered: %s\n", wp->NO, wp->expression);
      printf("Old value = " FMT_WORD "\n", wp->value);
      printf("New value = " FMT_WORD "\n", value);
      wp->value = value;
      nemu_state.state = NEMU_STOP;
    }
  }
}
