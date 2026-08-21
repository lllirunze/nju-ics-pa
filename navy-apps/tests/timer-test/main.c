#include <stdio.h>
#include <NDL.h>

int main() {
  NDL_Init(0);
  uint32_t next = NDL_GetTicks() + 500;
  while (1) {
    uint32_t now = NDL_GetTicks();
    if ((int32_t)(now - next) >= 0) {
      printf("0.5 second has passed\n");
      next += 500;
    }
  }

  return 0;
}
