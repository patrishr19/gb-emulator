CC = gcc
CFLAGS = -Wall -I"C:/msys64/mingw64/include" -Iinclude
LDFLAGS = -L"C:/msys64/mingw64/lib" -lraylib -lopengl32 -lgdi32 -lwinmm
SRC = src/main.c src/rom.c
OBJ = emulator.exe
all:
	$(CC) $(CFLAGS) $(SRC) -o $(OBJ) $(LDFLAGS)

clean:
	del $(OBJ)