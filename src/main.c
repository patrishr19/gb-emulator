#include <setup.h>
#include <emulator.h>
#include <rom.h>
#include <iogm.h>
#include <dma.h>
#include <ppu.h>
int main(int argc, char *argv[]) {
    Gameboy gb = {0};
    gb.bus.current_bank = 1;
    gb.bus.internal_divider = 0;
    CPUInit(&gb.cpu);
    ppu_init();
    IOInit(&gb.bus.io);

    bool running = false;

    if (argc < 2) {
        return 1;
    }

    if (LoadRom(&gb.bus, argv[1])) {
        printf("Loaded: %s\n", argv[1]);
        printf("Game: ");
        for (uint16_t i = 0x0134; i <= 0x0143; i++) { // 16 chars
            printf("%c", gb.bus.memory[i]);
        }
        printf("\n");
        running = true;
    } else {
        printf("Failed to load: %s\n", argv[1]);
        running = false;
    }
    
    while (running) {
        int cycles = CPUStep(&gb.cpu, &gb.bus);

        for (int i = 0; i < cycles; i += 4) {
            dma_tick(&gb.bus);
            TimerStep(&gb.bus, 4);
            ppu_tick(&gb.bus);
        }
    }

    return 0;
}
