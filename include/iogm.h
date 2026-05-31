/**
 * @file iogm.h
 * @brief  Memory mapped input/output registers
 * */
#pragma once
#include <setup.h>

/**
 * @brief Struct of I/O registers
 * */
typedef struct {
    uint8_t registers[256];
} IORegisters;


void IOInit(IORegisters *io);

/**
 * @brief Reads a byte value from IO register at an offset
 * @param io Pointer to the IO register context
 * @param offset The 8bit ofset
 * @return uint8_t The byte value fetched
 * */
uint8_t IORead(IORegisters *io, uint8_t offset);


/**
 * @brief Writes a byte value into an IO register at an offset
 * @param io Pointer to the IO register context
 * @param offset The 8bit ofset
 * @param value Byte to be written
 * */
void IOWrite(IORegisters *io, uint8_t offset, uint8_t value);
