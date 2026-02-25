#ifndef CPU_H
#define CPU_H

#include <setup.h>
#include <bus.h>
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
} CPU;


void CPUInit(CPU *cpu);

int CPUStep(CPU *cpu, Bus *bus);

void HandleInterrupt(CPU *cpu, Bus *bus, uint16_t handlerAddress, uint8_t interruptBit);

#endif