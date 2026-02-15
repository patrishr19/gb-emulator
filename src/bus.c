#include <setup.h>
#include <emulator.h>
#include <bus.h>
#include <iogm.h>

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

    if (address >= 0xFF00) {
        return IORead(&bus->io, address - 0xFF00);
    }
    return 0xFF;
    
}
void BusWrite(Bus *bus, uint16_t address, uint8_t value) {
    if (address >= 0xFF00) {
        IOWrite(&bus->io, address - 0xFF00, value);
        return;
    }

    if (address >= 0x0000 && address <= 0xFFFF) {
        bus->memory[address] = value;
    }
}
