#include <setup.h>
#include <emulator.h>
#include <rom.h>
int main() {
    Bus bus = {0};

    if (LoadRom(&bus, "testRoms/tetris.gb")) {
        printf("Game: ");
        for (uint16_t i = 0x0134; i <= 0x0143; i++) {
            printf("%c", bus.memory[i]);
        }
        printf("\n");
    }
   
    return 0;
}