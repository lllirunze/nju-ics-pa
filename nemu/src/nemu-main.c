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

#include <common.h>
#include <errno.h>
#include "monitor/sdb/sdb.h"

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

#ifndef CONFIG_TARGET_AM
const char *get_expr_test_file();

static int test_expr_file(const char *file_name) {
  FILE *fp = fopen(file_name, "r");
  if (fp == NULL) {
    perror(file_name);
    return 1;
  }

  char line[65536];
  int line_no = 0;
  while (fgets(line, sizeof(line), fp) != NULL) {
    line_no ++;
    char *end = NULL;
    errno = 0;
    unsigned long expected = strtoul(line, &end, 10);
    if (errno != 0 || end == line || *end != ' ') {
      printf("Invalid test case at line %d: %s", line_no, line);
      fclose(fp);
      return 1;
    }

    while (*end == ' ') end ++;
    end[strcspn(end, "\n")] = '\0';
    bool success = false;
    word_t actual = expr(end, &success);
    if (!success || actual != (word_t)expected) {
      printf("Expression test failed at line %d\n", line_no);
      printf("  expr: %s\n", end);
      printf("  expected: " FMT_WORD "\n", (word_t)expected);
      if (success) printf("  actual: " FMT_WORD "\n", actual);
      else printf("  actual: invalid expression\n");
      fclose(fp);
      return 1;
    }
  }

  if (ferror(fp)) {
    perror(file_name);
    fclose(fp);
    return 1;
  }
  fclose(fp);
  printf("Expression tests passed: %d cases\n", line_no);
  return 0;
}
#endif

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
  const char *expr_test_file = get_expr_test_file();
  if (expr_test_file != NULL) return test_expr_file(expr_test_file);
#endif

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
