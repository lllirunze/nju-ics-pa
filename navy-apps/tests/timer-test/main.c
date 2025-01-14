#include <stdio.h>
#include <sys/time.h>

int main() {
  struct timeval start, current;
  gettimeofday(&start, NULL);

  int count = 0;
  while (count < 5) {
    gettimeofday(&current, NULL);
    unsigned long totalMicroSeconds = (current.tv_sec - start.tv_sec) * 1000000 + (current.tv_usec - start.tv_usec);
    if (totalMicroSeconds >= 500000) {
      printf("Hello! This is a message every 0.5 seconds.\n");
      gettimeofday(&start, NULL);
      count++;
    }
  }
  
  return 0;
}