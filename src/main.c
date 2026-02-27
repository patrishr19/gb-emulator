#include <setup.h>
#include <emulator.h>
#include <rom.h>
#include <iogm.h>
int main(int argc, char *argv[]) {
    int stepCount = 0;
    const int MAX_STEPS = 10000000;

    int cycleCount = 0;
    const int CYCLES_PER_FRAME = 70224; //https://gbdev.io/pandocs/Rendering.html?highlight=70224#ppu-modes
    Gameboy gb = {0};

    CPUInit(&gb.cpu);

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
    
    while (running && stepCount < MAX_STEPS) {
        int cycles = CPUStep(&gb.cpu, &gb.bus);
        cycleCount += cycles;
        stepCount++;
        
        if (cycleCount >= CYCLES_PER_FRAME) {
            cycleCount -= CYCLES_PER_FRAME;
            

            //v-blank interrupt flag
            uint8_t ifFlags = BusRead(&gb.bus, 0xFF0F);
            // uint8_t ieFlags = BusRead(&gb.bus, 0xFFFF);

            if ((ifFlags & 0x01) == 0) {
                BusWrite(&gb.bus, 0xFF0F, ifFlags | 0x01);
            }
        }
        

        printf("next pc: 0x%04X | sp: 0x%04X, cycles: %d\n", gb.cpu.pc, gb.cpu.sp, cycles);
    }

    return 0;
}