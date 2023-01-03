#include "userapp.h"
#include <stdint.h> /* for uint64 definition */

#define NUM 20000
#define BILLION 1000000000L

void register_task(unsigned int pid, unsigned long period,
                   unsigned long processing_time) {
  char command[128];
  int ret = 0;
  ret = sprintf(command, "echo R %u %lu %lu > /proc/mp2/status", pid, period,
                processing_time);
  printf("%s\n", command);
  if (ret < 0) {
    printf("Something bad happened, during userspace register its pid\n");
  }
  system(command);
}

void yield_task(unsigned int pid) {
  char command[128];
  int ret = 0;
  ret = sprintf(command, "echo Y %u > /proc/mp2/status", pid);
  printf("%s\n", command);
  if (ret < 0) {
    printf("Something bad happened, during yielding the pid\n");
  }
  system(command);
}

void deregister(unsigned int pid) {
  char command[128];
  int ret = 0;

  ret = sprintf(command, "echo D %u > /proc/mp2/status", pid);
  printf("%s\n", command);
  if (ret < 0) {
    printf("Something bad happened, during deregistering the pid\n");
  }
  system(command);
}

void do_job(unsigned long processing_time) {
  // return;
  unsigned long i, j;
  clock_t start, end;
  int x = 1, factorial = 1;

  start = clock();
  end = start + (processing_time / 1000) * CLOCKS_PER_SEC;

  while (x <= NUM * 10000) {
    factorial = factorial * x;
    x += 1;
    if (clock() > end)
      break;
  }
}

int read_status() {
  char command[128];
  int ret = 0;
  ret = sprintf(command, "cat /proc/mp2/status");
  if (ret < 0) {
    printf("Something bad happened, during reading userspace register pid\n");
    return -1;
  }
  system(command);

  return 0;
}

int main(int argc, char *argv[]) {
  struct timespec initial, start, finish;
  unsigned long period = 0;
  unsigned long processing_time = 0;
  unsigned long num_jobs = 0;

  period = atoi(argv[1]);
  processing_time = atoi(argv[2]);
  num_jobs = atoi(argv[3]);

  // register pid
  unsigned int pid_t = getpid();
  register_task(pid_t, period, processing_time);

  clock_gettime(CLOCK_MONOTONIC_RAW, &initial);

  if (read_status() == -1) {
    return -1;
  }

  int i = 0;
  while (i < num_jobs) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    uint64_t wakeup_time = (start.tv_sec - initial.tv_sec) +
                           (start.tv_nsec - initial.tv_nsec) / BILLION;
    printf("wakeup_time is: %ld s\n", wakeup_time);

    do_job(processing_time);

    clock_gettime(CLOCK_MONOTONIC_RAW, &finish);
    uint64_t process_time = (finish.tv_sec - start.tv_sec) +
                            (finish.tv_nsec - start.tv_nsec) / BILLION;
    printf("process_time is: %ld s\n", process_time);

    yield_task(pid_t);
    i++;
  }

  deregister(pid_t);
  return 0;
}
