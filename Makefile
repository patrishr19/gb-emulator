
ifeq ($(OS), Windows_NT)
	TARGET = emulator.exe
	LDFLAGS = -lraylib -lgdi32 -lwinmm
	RM = del /Q
	CLEAN_OBJ = src\*.o
else
	TARGET = emulator
	LDFLAGS = -lm -lraylib -lGL -lpthread -ldl -lrt -lX11
	RM = rm -f
	CLEAN_OBJ = src/*.o
endif



CC = gcc
CFLAGS = -Wall -Iinclude -g
SRC = src/main.c src/rom.c src/cpu.c src/bus.c src/iogm.c src/cpu_ops.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(CLEAN_OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run