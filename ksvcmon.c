#include <ksvcmon.h>

int verbose = 0;

char *metrics_prefix = "ksvcmon";
struct metric *metrics[METRICS_MAX] = {0};
int metrics_count = 0;

void *metric_thread_main(void *arg)
{
    struct metric *m = (struct metric *)arg;
    while (1) {
        double value = m->do_once(m);

        pthread_mutex_lock(&m->lock);
        m->value = value;
        pthread_mutex_unlock(&m->lock);
        usleep(m->sleep_us);
    }
}

int metric_setup(struct metric *m)
{
    int ret;

    pthread_mutex_init(&m->lock, NULL);
    m->id = metrics_count;
    metrics[m->id] = m;
    metrics_count++;
    if (m->setup) {
        ret = m->setup(m);
        if (ret)
            return ret;
    }
    m->thread = NULL;

    if (!m->debug)
        return 0;
    printf("METRIC DEBUG TRIGGERED\n");
    printf("Metric %s registered\n", m->name);
    double value = m->do_once(m);
    printf("Metric %s value: %f\n", m->name, value);
    metric_teardown(m);
    exit(0);    
}

int metric_teardown(struct metric *m)
{
    int ret;
    if (m->teardown) {
        ret = m->teardown(m);
        if (ret)
            return ret;
    }
    if (m->thread) {
        pthread_cancel(*m->thread);
        pthread_join(*m->thread, NULL);
    }
    pthread_mutex_destroy(&m->lock);
    return 0;
}

void usage(char *name)
{
    fprintf(stderr, "Usage: %s\n", name);
    fprintf(stderr, "  -h HOST           host to bind the server to\n");
    fprintf(stderr, "  -p PORT           port to bind the server to\n");
    fprintf(stderr, "  -m METRICS_PREFIX prefix for the metrics (default: %s)\n", metrics_prefix);
    fprintf(stderr, "  -n                do not start the server\n");
    fprintf(stderr, "  -v                verbose mode - print metrics every second\n");
    fprintf(stderr, "  -l                list available metrics and exit\n");

}

int main(int argc, char *argv[])
{
    int opt;
    char *host = NULL;
    int port = -1;
    int start = 1;
    int list_metrics = 0;

    while ((opt = getopt(argc, argv, "lvnh:p:m:")) != -1) {
        switch (opt) {
            case 'l':
                list_metrics = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'n':
                start = 0;
                break;
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'm':
                metrics_prefix = optarg;
                break;
            case '?':
                if (optopt == 'h' || optopt == 'p' || optopt == 'm') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                }
                return EXIT_FAILURE;
            default:
                abort();
        }
    }

    if (list_metrics) {
        printf("Available metrics:\n");
        for (int i = 0; i < metrics_count; i++) {
            printf("%15s - %-50s\n", metrics[i]->name, metrics[i]->desc);
        }
        return 0;
    }

    if (start && (host == NULL || port == -1)) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (start && start_server(host, port)) {
        if (verbose)
            printf("Metrics prefix: %s\n", metrics_prefix);
        return 1;
    }

    for (int i = 0; i < metrics_count; i++) {
        metrics[i]->thread = malloc(sizeof(pthread_t));
        pthread_create(metrics[i]->thread, NULL, metric_thread_main, metrics[i]);
    }

    while (1) {
        if (!verbose) {
            usleep(1000 * 1000);
            continue;
        }
        for (int i = 0; i < metrics_count; i++) {
            pthread_mutex_lock(&metrics[i]->lock);
            printf("%-15s %14.4f\n", metrics[i]->name, metrics[i]->value);
            pthread_mutex_unlock(&metrics[i]->lock);
        }
        usleep(1000 * 1000);
    }

    if (start)
        stop_server();

    return 0;
}
