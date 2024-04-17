#ifndef KSVCMON_H
#define KSVCMON_H
#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define METRICS_MAX 100

extern int verbose;

struct metric {
    char            *name;
    char            *desc;
    double          value;
    int             sleep_us;
    double          (*do_once)(struct metric *); // shoots and sleeps for sleep_us
    int             (*setup)(struct metric *); // at load time
    int             (*teardown)(struct metric *); // at exit, may not be called
    int             debug;
//  --------------------------- PRIVATE PART ---
    int             id;
    pthread_mutex_t lock;
    pthread_t       *thread;
};

#define US_1_SEC    1000000

extern char *metrics_prefix;
extern struct metric *metrics[];
extern int metrics_count;

int metric_setup(struct metric *m);
int metric_teardown(struct metric *m);

int start_server(char *host, int port);
int stop_server();

#define REGISTER_METRIC(m_name) \
    __attribute__((constructor)) void __register_##m_name(void) { \
        if (metric_setup(&m_name)) \
            exit(1); \
    } \
    __attribute__((destructor)) void __unregister_##m_name(void) { \
        if (metric_teardown(&m_name)) \
            exit(1); \
    }

inline double time_diff_us(struct timespec *start, struct timespec *end)
{
    return (end->tv_sec - start->tv_sec) * 1e6 + (end->tv_nsec - start->tv_nsec) / 1e3;
}

inline void usleep(unsigned long usec)
{
    nanosleep(&(struct timespec){.tv_sec = usec / 1000000, .tv_nsec = (usec % 1000000) * 1000}, NULL);
}

#endif