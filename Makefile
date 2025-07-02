CC = gcc
CFLAGS = -std=c23 -Wall -Wextra -O0

SRC = main.c bitcask.c kv.c
OBJ = $(SRC:.c=.o)
TARGET = bitcask

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(TARGET) data
