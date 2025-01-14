#include <stdio.h>
#include <NDL.h>
#include <sys/time.h>

int main() {
  // struct timeval start, current;
  // gettimeofday(&start, NULL);

  // int count = 0;
  // while (count < 5) {
  //   gettimeofday(&current, NULL);
  //   unsigned long totalMicroSeconds = (current.tv_sec - start.tv_sec) * 1000000 + (current.tv_usec - start.tv_usec);
  //   if (totalMicroSeconds >= 500000) {
  //     printf("Hello! This is a message every 0.5 seconds.\n");
  //     gettimeofday(&start, NULL);
  //     count++;
  //   }
  // }

  // return 0;

  NDL_Init(0);
  uint32_t start_msec = NDL_GetTicks();
  uint32_t current_msec;
  int count = 0;
  while (count < 5) {
    current_msec = NDL_GetTicks();
    if (current_msec - start_msec >= 500) {
      printf("Hello! This is a message every 0.5 seconds.\n");
      start_msec = NDL_GetTicks();
      count++;
    }
  }
  NDL_Quit();

  return 0;

}