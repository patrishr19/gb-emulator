/**
 * @file cpu_prefix.h
 * @brief Handlers for prefixed opcodes
 * */
#pragma once

#include <setup.h>
#include <cpu.h>
/**
 * @brief Helper function to fetch values for prefix instructions
 * @param cpu Pointer to the cpu
 * @param bus Pointer to the bus
 * @param reg code index representing the target register
 * @return uint8_t byte value of the target
 * */
uint8_t get_cb_value(CPU *cpu, Bus *bus, uint8_t reg);
/***
 * @brief Function for setting values to the target register
 * @param cpu Pointer to the cpu
 * @param bus Pointer to the bus
 * @param reg target register
 * @param value the new value to be set in the target
 */
void set_cb_value(CPU *cpu, Bus *bus, uint8_t reg, uint8_t value);

