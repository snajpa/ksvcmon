#include <ksvcmon.h>
#include <sys/mman.h>

#define FILE_SIZE   4096
#define FILE_NAME   "/tmp/mmap_test"

int fd;

int metric_mmap_setup(struct metric *m)
{
    int ret;
    (void)m;

    fd = open(FILE_NAME, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        return fd;
    }

    ret = ftruncate(fd, FILE_SIZE);
    if (ret < 0) {
        close(fd);
        return ret;
    }

    return 0;
}

int metric_mmap_teardown(struct metric *m)
{
    (void)m;

    close(fd);
    unlink(FILE_NAME);

    return 0;
}

double metric_mmap_do(struct metric *m)
{
    struct timespec start, end;
    (void)m;

    clock_gettime(CLOCK_MONOTONIC, &start);
    void *addr = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        close(fd);
        return -1;
    }
    munmap(addr, FILE_SIZE);
    clock_gettime(CLOCK_MONOTONIC, &end);

    return time_diff_us(&start, &end);
}

struct metric metric_mmap = {
    .name = "mmap",
    .desc = "mmap and munmap a file",
    .do_once = metric_mmap_do,
    .setup = metric_mmap_setup,
    .teardown = metric_mmap_teardown,
    .sleep_us = US_1_SEC,
};

REGISTER_METRIC(metric_mmap)