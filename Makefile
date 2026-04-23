CC = cc
CFLAGS = -Wall -Wextra -std=c11
TARGET = hpool
SRCS = hpool.c hp_palloc.c
OBJS = $(SRCS:.c=.o)

.PHONY: clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c hp_palloc.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
