/**
 * @file bus.h
 * @brief Memory bus for reading, writing and timing
 * */
#pragma once

#include <setup.h>
#include <iogm.h>
/**
 * @brief Represents the memory bus, with ROM banks and I/O registers
*/
typedef struct{
    uint8_t memory[0x200000];
    IORegisters io;
    uint16_t internal_divider;
    uint8_t current_bank;
    uint8_t bank_upper;
    uint8_t banking_mode;
    uint8_t ram_enabled;
} Bus;

uint8_t BusRead(Bus *bus, uint16_t address);
uint16_t BusRead16(Bus *bus, uint16_t address);
void BusWrite(Bus *bus, uint16_t address, uint8_t value);

/**
 *	@brief Steps the system timer by number of CPU cycles
 * */
void TimerStep(Bus *bus, int cycles);

