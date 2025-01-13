#include <stdio.h>
#include <sys/time.h>

int main() {
  struct timeval start, current;
    gettimeofday(&start, NULL);

    while (1) {
        gettimeofday(&current, NULL);

        // 计算时间差（以秒为单位）
        double elapsed_time = (current.tv_sec - start.tv_sec) +
                              (current.tv_usec - start.tv_usec) / 1000000.0;

        // 每0.5秒输出一句话
        if (elapsed_time >= 0.5) {
            printf("Hello! This is a message every 0.5 seconds.\n");
            // fflush(stdout); // 确保输出立即刷新到终端

            // 更新起始时间为当前时间
            gettimeofday(&start, NULL);
        }

        // 防止占用过高的 CPU，适当休眠
        // usleep(10000); // 休眠10毫秒
    }
  return 0;
}