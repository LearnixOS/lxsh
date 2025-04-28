CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lreadline
SRC = src/main.c src/builtins.c src/execute.c src/utils.c src/jobs.c src/history.c src/config.c
OBJ = $(SRC:.c=.o)
TARGET = lxsh

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
