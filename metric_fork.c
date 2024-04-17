#include <ksvcmon.h>

double do_metric_fork(struct metric *m)
{
    struct timespec start, end;
    (void)m;

    clock_gettime(CLOCK_MONOTONIC, &start);

    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }
    if (pid == 0) {
        exit(0);
    }
    waitpid(pid, NULL, 0);
    
    clock_gettime(CLOCK_MONOTONIC, &end);

    return time_diff_us(&start, &end);
}

struct metric metric_fork = {
    .name = "fork",
    .desc = "fork and waitpid",
    .do_once = do_metric_fork,
    .sleep_us = US_1_SEC,
};

REGISTER_METRIC(metric_fork)