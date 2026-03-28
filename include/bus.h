#ifndef BUS_H
#define BUS_H

#include <setup.h>
#include <iogm.h>

typedef struct{
    // 64kb
    uint8_t memory[0x10000];
    IORegisters io;
    uint16_t internal_divider;
} Bus;

uint8_t BusRead(Bus *bus, uint16_t address);
uint16_t BusRead16(Bus *bus, uint16_t address);
void BusWrite(Bus *bus, uint16_t address, uint8_t value);
void TimerStep(Bus *bus, int cycles);

#endif