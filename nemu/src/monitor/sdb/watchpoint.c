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
#include <string.h>

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expression[1024];
  word_t old_val;

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
    printf("Error: There is no available watchpoint.\n");
    return NULL;
  }
  WP *wp = free_;
  free_ = free_->next;
  return wp;
}

// static void free_wp(WP *wp) {
//   // Here I didn't return wp to free_.
//   // free(wp->expression);
//   // wp->val = 0;
//   return;
// }

void display_wp() {
  if (head == NULL) {
    printf("No watchpoints.\n");
    return;
  }
  WP *cur = head;
  printf("Num\tExpr\tValue\n");
  while (cur != NULL) {
    printf("%d\t%s\t%u\n", cur->NO, cur->expression, cur->old_val);
    cur = cur->next;
  }
  return;
}

int set_wp(char* args) {
  bool success = true;
  word_t result = expr(args, &success);
  if (success == false) {
    printf("Error: Unable to calculate correctly.\n");
    return -1;
  }

  WP *wp = new_wp();
  if (wp == NULL) {
    printf("Error: Unable to set a watchpoint.\n");
    return -1;
  }

  strcpy(wp->expression, args);
  // wp->expression = str;
  wp->old_val = result;
  wp->next = head;
  head = wp;
  return wp->NO;
}

void delete_wp(int n) {
  if (n < 0 || n >= NR_WP) {
    printf("Error: Invalid value of arguments, and N need to be between 0 and 31.\n");
    return;
  }

  bool find_wp = false;
  // WP *wp;

  WP *cur = head;
  if (cur->NO == n) {
    find_wp = true;
    // wp = cur;
    head = head->next;
  }
  else {
    while (cur->next != NULL) {
      if (cur->next->NO == n) {
        find_wp = true;
        // wp = cur->next;
        cur->next = cur->next->next;
        break;
      }
      cur = cur->next;
    }
  }
  
  if (find_wp == false) {
    printf("Usage: Unable to find watchpoint %d\n", n);
  }
  else {
    printf("Delete watchpoint %d\n", n);
    // free_wp(wp);
  }
  return;
}

bool scan_wp(vaddr_t pc) {
  // printf("scan once.\n");
  bool check = false;
  bool success;
  WP *cur = head;
  while (cur != NULL) {
    // printf("%d\t%s\t%d\n", cur->NO, cur->expression, cur->old_val);
    success = true;
    word_t new_val = expr(cur->expression, &success);
    if (cur->old_val != new_val) {
      check = true;
      printf("%d\t%s\t%u\t%u\n", cur->NO, cur->expression, cur->old_val, new_val);
    }
    cur->old_val = new_val;
    cur = cur->next;
  }

  return check;
}

