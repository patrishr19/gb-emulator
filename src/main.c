#include <setup.h>
#include <emulator.h>
#include <rom.h>
int main() {
    Gameboy gb = {0};

    CPUInit(&gb.cpu);

    bool running = false;

    // printf("CPU init done, pc register at: 0x%04X\n", cpu.pc);

    if (LoadRom(&gb.bus, "testRoms/tetris.gb")) {
        printf("Game: ");
        for (uint16_t i = 0x0134; i <= 0x0143; i++) { // 16 chars
            printf("%c", gb.bus.memory[i]);
        }
        printf("\n");

        running = true;
    } else {
        running = false;
    }

    while (running) {
        // getchar();
        
        int cycles = CPUStep(&gb.cpu, &gb.bus);

        printf("next pc: 0x%04X | sp: 0x%04X, cycles: %d\n", gb.cpu.pc, gb.cpu.sp, cycles);
    }

    return 0;
}