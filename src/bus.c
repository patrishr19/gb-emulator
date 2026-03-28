#include <setup.h>
#include <emulator.h>
#include <bus.h>
#include <iogm.h>

uint8_t BusRead(Bus *bus, uint16_t address) {
    if (address == 0xFF04) {
        return (bus->internal_divider >> 8);
    }
    
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
    if (address == 0xFF04) {
        bus->internal_divider = 0;
        bus->io.registers[0x04] = 0;
        return;
    }
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

void TimerStep(Bus *bus, int cycles) {
    uint8_t tac = bus->io.registers[0x07];
    int bit = 0;
    switch (tac & 0x03) {
        case 0: bit = 9; break;
        case 1: bit = 3; break;
        case 2: bit = 5; break;
        case 3: bit = 7; break;
    }

    for (int i = 0; i < cycles; i++) {
        bool enabled = (tac & 0x04) != 0;
        bool signal_before = enabled && ((bus->internal_divider >> bit) & 1);

        bus->internal_divider++;
        
        bool signal_after = enabled && ((bus->internal_divider >> bit) & 1);

        if (signal_before && !signal_after) {
            if (bus->io.registers[0x05] == 0xFF) {
                bus->io.registers[0x05] = bus->io.registers[0x06];
                bus->io.registers[0x0F] |= 0x04;
            } else {
                bus->io.registers[0x05]++;
            }
        }
    }
    bus->io.registers[0x04] = (bus->internal_divider >> 8);
}