#include <setup.h>
#include <emulator.h>
#include <bus.h>
#include <iogm.h>

uint8_t BusRead(Bus *bus, uint16_t address) {
    if (address >= 0xE000 && address <= 0xFDFF) {
        address -= 0x2000;
    }

    if (address >= 0xFF00 && address < 0xFFFF) {
        return IORead(&bus->io, address - 0xFF00);
    }

    if (address == 0xFFFF) {
        return IORead(&bus->io, 0xFF);
    }

    return bus->memory[address]; //for now
    
}
void BusWrite(Bus *bus, uint16_t address, uint8_t value) {
    if (address == 0xFF01) {
        printf("%c", value);
        fflush(stdout);
    }

    if (address < 0x8000) {
        //TODO MBC
        return;
    }

    if (address >= 0xE000 && address <= 0xFDFF) {
        address -= 0x2000;
    }

    // io registers
    if (address >= 0xFF00 && address < 0xFFFF) {
        IOWrite(&bus->io, address - 0xFF00, value);
        return;
    }

    // ei register
    if (address == 0xFFFF) {
        IOWrite(&bus->io, 0xFF, value);
        return;
    }
    
    bus->memory[address] = value;
}

uint16_t BusRead16(Bus *bus, uint16_t address) {
    uint16_t low = BusRead(bus, address);
    uint16_t high = BusRead(bus, address + 1);
    return (high << 8) | low;
}
