ksvcmon - Kernel Service Response Time Monitor
==============================================

This is a simple tool to monitor the response time of various kernel services. It also provides a simple Prometheus exporter to expose the metrics.

## Dependencies

```shell
$ sudo apt install libmicrohttpd-dev build-essential make
```

## Running

```shell
$ make -j
$ ./ksvcmon
Usage: ./ksvcmon
  -h HOST           host to bind the server to
  -p PORT           port to bind the server to
  -m METRICS_PREFIX prefix for the metrics (default: ksvcmon)
  -n                do not start the server
  -v                verbose mode - print metrics every second
  -l                list available metrics and exit
```

## Available Metrics

```shell
$ ./ksvcmon -l
Available metrics:
           fork - fork and waitpid                                  
           mmap - mmap and munmap a file                            
        pthread - pthread_create and pthread_join                   
      rtnetlink - get interface addresses over netlink
```