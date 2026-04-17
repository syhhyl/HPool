CC = clang
CFLAGS = -Wall -Wextra

TARGET = hpool
SRC = hp_palloc.c hpool.c
OBJ = $(SRC:.c=.o)


all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)


