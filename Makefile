CC = gcc

CFLAGS = -Wall -Wextra -g

TARGET = main

SRCS = main.c simple_malloc.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rm malloc_log.txt
