#include <setup.h>
#include <emulator.h>
#include <cpu.h>
#include <bus.h>

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
    printf("Opcode at 0x%04X: 0x%02X\n", cpu->pc, opcode);
    cpu->pc++;
    switch(opcode) { // opcodes
        case 0x00:
            return 4;
        case 0xC3: { // JP NZ, a16    3  16/12 - - - -
            uint16_t address = BusRead(bus, cpu->pc);
            address |= BusRead(bus, cpu->pc + 1) << 8;
            cpu->pc += 2;
            
            // Z
            if ((cpu->f & 0x80) == 0) {
                cpu->pc = address;
                return 16;
            }
            return 12;
        }
        case 0xCE: { //ADC A, n8   2 8    Z 0 H C
            uint8_t value = BusRead(bus, cpu->pc);
            cpu->pc++;

            uint16_t result = cpu->a + value + (cpu->f & 0x10 ? 1 : 0); // 0x10 is 4 bit and flag C carry is on bit 4

            cpu->a = result & 0xFF; // bitmask 1111 1111 to 8bit

            // Z
            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            // N
            cpu->f &= ~0x40; // 6 bit
            
            // C
            if (result > 0xFF) {
                cpu->f |= 0x10;
            } else {
                cpu->f &= ~0x10;
            }

            // H
            if ((cpu->a ^ value ^ result) & 0x10) { // overflow from carry
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            return 8;
        }
        default:
            printf("Crash: opcode 0x%02X at pc 0x%04X\n", opcode, cpu->pc - 1);
            exit(1);
    }
}
