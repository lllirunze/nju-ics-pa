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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static inline int choose(int n) {
  return rand() % n;
}

static inline void gen(char e) {
  int i = 0;
  while (buf[i] != '\0') i++;
  buf[i] = e;
  buf[i+1] = '\0';
  return;
}

static inline void gen_num() {
  uint32_t num = ((uint32_t) rand()) % 10000;
  sprintf(buf+strlen(buf), "%d", num);
  return;
}

static inline void gen_op() {
  switch(choose(4)) {
    case 0: gen('+'); break;
    case 1: gen('-'); break;
    case 2: gen('*'); break;
    case 3: gen('/'); break;
    default: break;
  }
}

static void gen_rand_expr() {
  // buf[0] = '\0';
  char *s = buf;
  if (strlen(buf) > 0 && *(buf+strlen(buf)-1) == '/') {
    s = buf+strlen(buf);
  }
  int n = choose(3);
  switch(n) {
    case 0: gen_num(); break;
    case 1: 
      gen('(');
      gen_rand_expr();
      gen(')');
      break;
    default:
      gen_rand_expr();
      gen_op();
      gen_rand_expr();
      break;
  }

  // test division 0
  if (s != buf) {
    sprintf(code_buf, code_format, s);
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if(ret != 0){
			printf("ret: %d\n",ret);
		}
		
		fp = popen("/tmp/.expr", "r");
		assert(fp != NULL);

		int result;
		fscanf(fp, "%d", &result);
		pclose(fp);
		
		if(result == 0){
			memset((void*)s, 0, strlen(s) * sizeof(char));
			gen_rand_expr();
		}

  }

}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
