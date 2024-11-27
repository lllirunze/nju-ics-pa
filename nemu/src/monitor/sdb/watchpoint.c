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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char *expression;
  word_t val;

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

/* TODO: Implement the functionality of watchpoint */

static WP *new_wp() {
  if (free_ == NULL) {
    printf("Error: There is no enough watchpoint.\n");
    return NULL;
  }
  WP *wp = free_;
  free_ = free_->next;
  return wp;
}

void free_wp(WP *wp) {

}

void display_wp() {

}

int set_wp(char* args) {
  bool success = true;
  word_t result = expr(args, &success);
  if (success == false) {
    printf("Error: Unable to calculate correctly.\n");
    return 0;
  }

  WP *wp = new_wp();
  if (wp == NULL) {
    printf("Error: Unable to set a watchpoint.\n");
    return 0;
  }

  strcpy(wp->expression, args);
  wp->val = result;
  wp->next = head;
  head = wp;

  return 0;
}

