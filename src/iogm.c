#include <iogm.h>
#include <setup.h>
#include <bus.h>

void IOInit(IORegisters *io) {
    for (int i = 0;i < 256; i++) {
        io->registers[i] = 0;
    }

    io->registers[0x44] = 0x94;
    io->registers[0x40] = 0x91;
    io->registers[0xFF] = 0x00;
}

uint8_t IORead(IORegisters *io, uint8_t offset) {
    if (offset == 0x0F) {
        return io->registers[offset] & 0x1F;
    }
    
    return io->registers[offset];
}

void IOWrite(IORegisters *io, uint8_t offset, uint8_t value) {
    if (offset == 0x0F) {
        printf(">>> IOWrite IF: offset=0x%02X, value=0x%02X\n", offset, value);
        io->registers[offset] = value & 0x1F;
        printf(">>> After write, IF=0x%02X\n", io->registers[offset]);
    } else if (offset == 0xFF) {
        io->registers[offset] = value & 0x1F;
    } else {
        io->registers[offset] = value;
    }
}