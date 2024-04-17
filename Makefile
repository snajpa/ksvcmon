PROG=ksvcmon

CC=gcc
LIBS=-lpthread -lmicrohttpd
INCLUDES=-I.
CFLAGS=-Wall -Wextra -Werror -pedantic -std=c99 -g -O2 $(INCLUDES) $(LIBS)

C_FILES=$(wildcard *.c)
H_FILES=$(wildcard *.h)
O_FILES=$(C_FILES:.c=.o)

default: $(PROG)

%.o: %.c $(H_FILES)
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROG): $(O_FILES)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(PROG) $(O_FILES)

.PHONY: clean
