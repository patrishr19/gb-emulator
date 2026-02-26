#include <setup.h>
#include <emulator.h>
#include <bus.h>
#include <iogm.h>

uint8_t BusRead(Bus *bus, uint16_t address) {
    // ROM 0x0000 - 0x7FFF
    // if (address >= 0x0000 && address <= 0x7FFF) {
    //     return bus->memory[address];
    // }

    // VRAM
    /*
        if (address >= 0x8000 && address <= 0x9FFF) {
            return 0;
        }
    */
    if (address == 0xFFFF) {
        return IORead(&bus->io, 0xFF);
    }
    if (address >= 0xFF00) {
        return IORead(&bus->io, address - 0xFF00);
    }
    return bus->memory[address]; //for now
    
}
void BusWrite(Bus *bus, uint16_t address, uint8_t value) {
    if (address == 0xFF01) {
        printf("%c", value);
        fflush(stdout);
    }

    // ei register
    if (address == 0xFFFF) {
        IOWrite(&bus->io, 0xFF, value);
        return;
    }

    // io registers
    if (address >= 0xFF00) {
        IOWrite(&bus->io, address - 0xFF00, value);
        return;
    }
    // TODO rom protection
    if (address < 0x8000) {
        return; 
    }

    bus->memory[address] = value;
    
}

uint16_t BusRead16(Bus *bus, uint16_t address) {
    uint16_t low = BusRead(bus, address);
    uint16_t high = BusRead(bus, address + 1);
    return (high << 8) | low;
}
