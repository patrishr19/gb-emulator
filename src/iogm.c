#include <iogm.h>
#include <setup.h>
#include <bus.h>

void IOInit(IORegisters *io) {
    for (int i = 0;i < 256; i++) {
        io->registers[i] = 0;
    }

    io->registers[0x44] = 0x94;
    io->registers[0x40] = 0x91;
}

uint8_t IORead(IORegisters *io, uint8_t offset) {
    return io->registers[offset];
}

void IOWrite(IORegisters *io, uint8_t offset, uint8_t value) {
    io->registers[offset] = value;
}