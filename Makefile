CC = gcc
CFLAGS = -std=c23 -Wall -Wextra -O2

SRC = main.c bitcask.c kv.c
OBJ = $(SRC:.c=.o)
TARGET = bitcask

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
