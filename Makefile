CC = gcc
CFLAGS = -Wall -Iinclude -g
LDFLAGS = -lm -lraylib -lGL -lpthread -ldl -lrt -lX11

SRC = src/main.c src/rom.c src/cpu.c src/bus.c src/iogm.c
OBJ = $(SRC:.c=.o)
TARGET = emulator

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run