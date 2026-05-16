#pragma once

#include <setup.h>
#include <cpu.h>

uint8_t get_cb_value(CPU *cpu, Bus *bus, uint8_t reg);

void set_cb_value(CPU *cpu, Bus *bus, uint8_t reg, uint8_t value);

