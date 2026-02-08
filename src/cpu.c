#include <setup.h>
#include <emulator.h>
#include <cpu.h>


void CPUInit(CPU *cpu) { // this skips the nintendo logo and boot up sequence
    cpu->a = 0x01;
    cpu->f = 0xB0;

    cpu->b = 0x00;
    cpu->c = 0x13;

    cpu->d = 0x00;
    cpu->e = 0xD6;
    
    cpu->h = 0x01;
    cpu->l = 0x4D;

    cpu->pc = 0x0100;
    cpu->sp = 0xFFFE;
}

int CPUStep(CPU *cpu, Bus *bus) {
    uint8_t opcode = BusRead(bus, cpu->pc);
    printf("pc: %d\n", cpu->pc);
    cpu->pc++;
    printf("pc: %d\n", cpu->pc);

    switch(opcode) { // opcodes
        case 0x00:
            return 4;
        default:
            printf("Crash: opcode 0x%02X at pc 0x%04X\n", opcode, cpu->pc - 1);
            exit(1);
    }
}
