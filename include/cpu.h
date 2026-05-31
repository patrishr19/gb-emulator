/**
 * @file cpu.h
 * @brief CPU structure definitions, registers, flags and state
 * */

#pragma once

#include <setup.h>
#include <bus.h>

#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10
/**
 * @brief Represents the actual CPU struct with the registers
 * */
typedef struct {
    // register pairs https://gbdev.io/pandocs/CPU_Registers_and_Flags.html

    union {
        struct {
            uint8_t f;
            uint8_t a;
        };
        uint16_t af;
    };

    union {
        struct {
            uint8_t c;
            uint8_t b;
        };
        uint16_t bc;
    };

    union {
        struct {
            uint8_t e;
            uint8_t d;
        };
        uint16_t de;
    };

    union {
        struct {
            uint8_t l;
            uint8_t h;
        };
        uint16_t hl;
    };

    uint16_t sp;
    uint16_t pc;

    uint8_t ime;
    uint8_t halt;
    uint8_t ime_scheduled;
} CPU;

void CPUInit(CPU *cpu);

/**
 * @brief Executes a single opcode
 * */
int CPUStep(CPU *cpu, Bus *bus);

/**
 * @brief Handles a hardware interrput by jumping to a new location
 * @param handlerAddress 16bit memory address of the interrupt
 * @param interruptBit The bitmasked value
 * */
void HandleInterrupt(CPU *cpu, Bus *bus, uint16_t handlerAddress, uint8_t interruptBit);

void flagSet(CPU *cpu, uint8_t flag, bool value);

bool flagGet(CPU *cpu, uint8_t flag);

