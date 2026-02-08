#include <setup.h>
#include <emulator.h>
#include <bus.h>

uint8_t BusRead(Bus *bus, uint16_t address) {
    // ROM 0x0000 - 0x7FFF
    if (address >= 0x0000 && address <= 0x7FFF) {
        return bus->memory[address];
    }

    // VRAM
    /*
        if (address >= 0x8000 && address <= 0x9FFF) {
            return 0;
        }
    */

    return 0xFF;
    
}