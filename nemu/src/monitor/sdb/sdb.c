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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  char *str = strtok(args, " ");
  if (str == NULL) {
    // printf("No arguments.\n");
    cpu_exec(1);
  }
  else {
    uint64_t step = strtoul(str, NULL, 10);
    // printf("%lu\n", step);
    cpu_exec(step);
  }

  return 0;
}

static int cmd_info(char *args) {

  // printf("Print out register status or watchpoints.\n");
  char *str = strtok(args, " ");
  if (str == NULL) {
    printf("Error: Invalid number of arguments.\n");
    printf("Usage: info <SUBCMD>\n");
    return 0;
  }
  if (strcmp(str, "r") == 0) {
    isa_reg_display();
  }
  else if (strcmp(str, "w") == 0) {
    // todo: print out watchpoints
    printf("I want to print out watchpoints.\n");
  }
  else {
    printf("Error: Invalid format of arguments.\n");
    printf("Usage: info <r|w>\n");
  }

  return 0;
}

static int cmd_x(char *args) {
  char *str = strtok(NULL, " ");
  if (str == NULL) {
    printf("Error: Invalid format of arguments.\n");
    printf("Usage: x N EXPR\n");
    return 0;
  }
  int N = atoi(str);
  if (N <= 0) {
    printf("Error: Invalid value of arguments, and N need to be greater than 0.\n");
    return 0;
  }
  // printf("%d\n", N);

  char *expr = strtok(NULL, " ")+2;
  if (expr == NULL) {
    printf("Error: Invalid format of arguments.\n");
    printf("Usage: x N EXPR\n");
    return 0;
  }
  // printf("%s\n", expr);
  vaddr_t addr = (unsigned int)strtoul(expr, NULL, 16);
  // printf("%08x\n", addr);
  int i;
  for (i = 0; i < N; i++) {
    word_t desc = vaddr_read(addr, 4);
    printf("0x%08x\t0x%04x\n", addr, desc);
    addr += 4;
  }

  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Execute the program step by step", cmd_si },
  { "info", "Print out register status or watchpoints", cmd_info },
  { "x", "Scan memory and output N consecutive 4-byte characters in hex format", cmd_x },

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
