#include <ksvcmon.h>
#include <sys/mman.h>

void *do_nothing(void *arg)
{
    (void)arg;
    return NULL;
}

double do_metric_pthread(struct metric *m)
{
    int ret;
    (void)m;
    struct timespec start, end;
    pthread_t thread;

    clock_gettime(CLOCK_MONOTONIC, &start);

    ret = pthread_create(&thread, NULL, do_nothing, NULL);
    if (ret < 0) {
        return -1;
    }

    pthread_join(thread, NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    return time_diff_us(&start, &end);
}

struct metric metric_pthread = {
    .name = "pthread",
    .desc = "pthread_create and pthread_join",
    .do_once = do_metric_pthread,
    .sleep_us = US_1_SEC,
};

REGISTER_METRIC(metric_pthread)