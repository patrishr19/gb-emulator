#ifndef CPU_PREFIX_H
#define CPU_PREFIX_H
#include <setup.h>
#include <cpu.c>

int execute_cb(CPU *cpu, Bus *bus, uint8_t opcode);

#endif