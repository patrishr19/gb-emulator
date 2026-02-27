#ifndef CPU_OPS_H
#define CPU_OPS_H

#include "cpu.h"
#include "bus.h"

void op_xor(CPU *cpu, uint8_t value);

void op_and(CPU *cpu, uint8_t value);

void op_or(CPU *cpu, uint8_t value);

uint8_t op_inc_8(CPU *cpu, uint8_t value);
uint16_t op_inc_16(CPU *cpu, uint16_t value);

uint8_t op_dec_8(CPU *cpu, uint8_t value);
uint16_t op_dec_16(CPU *cpu, uint16_t value);

void op_jr(CPU *cpu, Bus *bus);

void op_jp(CPU *cpu, Bus *bus);

uint8_t op_ld_immediate(CPU *cpu, Bus *bus);

uint8_t op_add(CPU *cpu, uint8_t a, uint8_t b, bool useCarry);

uint8_t op_sub(CPU *cpu, uint8_t a, uint8_t b, bool useCarry);

void op_add_hl(CPU *cpu, uint16_t value);

void op_call(CPU *cpu, Bus *bus);

void op_ret(CPU *cpu, Bus *bus);

void op_rst(CPU *cpu, Bus *bus, uint16_t address);
#endif