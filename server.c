#include <ksvcmon.h>
#include <microhttpd.h>
#include <arpa/inet.h>
       
#define PORT 8888
#define HOST "0.0.0.0"

size_t out_metrics(char *buf, size_t *size)
{
    size_t offset = 0;
    for (int i = 0; i < metrics_count; i++) {
        if (offset + 128*1024 > *size) {
            buf = realloc(buf, *size * 2);
        }
        pthread_mutex_lock(&metrics[i]->lock);
        offset += snprintf(buf + offset, *size - offset, "# HELP %s_%s %s\n", metrics_prefix, metrics[i]->name, metrics[i]->desc);
        offset += snprintf(buf + offset, *size - offset, "# TYPE %s_%s gauge\n", metrics_prefix, metrics[i]->name);
        offset += snprintf(buf + offset, *size - offset, "%s_%s %f\n", metrics_prefix, metrics[i]->name, metrics[i]->value);
        pthread_mutex_unlock(&metrics[i]->lock);
    }
    return offset;
}

enum MHD_Result
answer_to_connection(void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size, void **ptr)
{
    struct MHD_Response *rsp;
    int ret;
    char *buf;
    size_t buf_size = 1024*1024;
    (void)cls;
    (void)url;
    (void)version;
    (void)upload_data;
    (void)upload_data_size;
    (void)ptr;

    if (strcmp(method, "GET") != 0) {
        buf = "Invalid HTTP Method\n";
        rsp = MHD_create_response_from_buffer(strlen(buf), (void *)buf, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, rsp);
        MHD_destroy_response(rsp);
        return ret;
    }
    if (strcmp(url, "/") == 0) {
        for (int i = 0; i < metrics_count; i++) {
            pthread_mutex_lock(&metrics[i]->lock);
            printf("%s: %f\n", metrics[i]->name, metrics[i]->value);
            pthread_mutex_unlock(&metrics[i]->lock);
        }
        buf = "OK\n";
        rsp = MHD_create_response_from_buffer(strlen(buf), (void *)buf, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_OK, rsp);
        MHD_destroy_response(rsp);
        return ret;
    }
    if (strcmp(url, "/metrics") == 0) {
        buf = malloc(buf_size);
        if (!buf) {
            return MHD_NO;
        }
        ret = out_metrics(buf, &buf_size);
        rsp = MHD_create_response_from_buffer(strlen(buf), (void *)buf, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_OK, rsp);
        MHD_destroy_response(rsp);
        free(buf);
        return ret;
    }
    buf = "Bad Request\n";
    rsp = MHD_create_response_from_buffer(strlen(buf), (void *)buf, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, rsp);
    MHD_destroy_response(rsp);
    return ret;

    if (!rsp) {
        return MHD_NO;
    }

    ret = MHD_queue_response(connection, MHD_HTTP_OK, rsp);
    MHD_destroy_response(rsp);

    return ret;
}

struct MHD_Daemon *daemon;

int start_server(char *host, int port)
{
    printf("Starting server on %s:%d\n", host, port);
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_SOCK_ADDR, inet_addr(host),
                              MHD_OPTION_END);
    if (!daemon) {
        printf("Cannot start server\n");
        return -1;
    }

    printf("Server started on %s:%d\n", host, port);
    return 0;
}

int stop_server()
{
    MHD_stop_daemon(daemon);
    return 0;
}