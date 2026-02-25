#ifndef IOGM_H
#define IOGM_H

#include <setup.h>

typedef struct {
    uint8_t registers[256];
}IORegisters;


void IOInit(IORegisters *io);
uint8_t IORead(IORegisters *io, uint8_t offset);
void IOWrite(IORegisters *io, uint8_t offset, uint8_t value);
#endif