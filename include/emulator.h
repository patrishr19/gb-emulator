#ifndef EMULATOR_H
#define EMULATOR_H

#include <setup.h>
#include <cpu.h>
#include <bus.h>

typedef struct{
    CPU cpu;
    Bus bus;
} Gameboy;

#endif