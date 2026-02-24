#include <setup.h>
#include <emulator.h>
#include <cpu.h>
#include <bus.h>
#include <iogm.h>

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

    cpu->ime = 1;
}

int CPUStep(CPU *cpu, Bus *bus) {
    uint8_t opcode = BusRead(bus, cpu->pc);
    printf("Opcode at 0x%04X: 0x%02X\n", cpu->pc, opcode);

    
    cpu->pc++;
    switch(opcode) { // opcodes
        case 0x00: {
            return 4;
        }
        case 0xC3: { // JP a16    3  16    - - - -
            uint16_t address = BusRead(bus, cpu->pc);
            address |= BusRead(bus, cpu->pc + 1) << 8;
            cpu->pc += 2;
            
            cpu->pc = address;
            return 16;
        }
        case 0xCE: { //ADC A, n8   2 8    Z 0 H C
            uint8_t value = BusRead(bus, cpu->pc);
            cpu->pc++;
            uint8_t originA = cpu->a;

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
            if ((((originA & 0x0F) + (value & 0x0F) + (cpu->f & 0x10 ? 1 : 0)) & 0x10) != 0) { // overflow from carry
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            return 8;
        }
        case 0x66: {
            uint16_t address = (cpu->h << 8) | cpu->l; // h- high byte, l- low byte, shift the high byte 8 places
            uint8_t value = BusRead(bus, address);
            cpu->h = value;
            return 8;
        }
        case 0xCC: {
            uint16_t address = BusRead(bus, cpu->pc);
            address |= BusRead(bus, cpu->pc + 1) << 8; // cpu->pc low byte cpu->pc + 1 high byte
            cpu->pc += 2; // a16

            // Z
            if ((cpu->f & 0x80) != 0) {
                cpu->sp -= 2; // space for 2 bytes

                BusWrite(bus, cpu->sp, cpu->pc & 0xFF);
                BusWrite(bus, cpu->sp + 1, (cpu->pc >> 8) & 0xFF);
                cpu->pc = address;
                return 24;
            }
            return 12;
        }
        case 0x0B: { // DEC BC   1  8   - - - - 
            cpu->bc--;
            return 8;
        }
        case 0x03: { // INC BC   1 8 -----
            cpu->bc++;
            return 8;
        }
        case 0x73: { // LD [HL], E    1  8 - - - -
            uint16_t address = (cpu->h << 8) | cpu->l;
            BusWrite(bus, address, cpu->e);
            return 8;
        }
        case 0x83: { // ADD A, E    1  4     Z 0 H C
            uint16_t result = cpu->a + cpu->e;
            uint8_t originA = cpu->a;
            cpu->a = result & 0xFF;
            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;

            if ((((originA & 0x0F) + (cpu->e & 0x0F)) & 0x10) != 0) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            if (result > 0xFF) { // 0000 0000 1111 1111 "overflow"
                cpu->f |= 0x10;
            } else {
                cpu->f &= ~0x10;
            }

            return 4;
        }
        case 0x0C: { // INC C     1  4    Z 0 H -
            uint8_t result = cpu->c + 1;
            uint8_t originC = cpu->c;
            cpu->c = result & 0xFF;

            if (cpu->c == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;

            if ((((originC & 0x0F) + 1) & 0x10) != 0) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            return 4;
        }
        case 0x0D: { // DEC C    1  4    Z 1 H -
            uint8_t result = cpu->c - 1;
            uint8_t originC = cpu->c;
            cpu->c = result & 0xFF;

            if (cpu->c == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f |= 0x40;

            if ((((originC & 0x0F) - 1) & 0x10) != 0) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            return 4;
        }
        case 0x08: { // LD [a16], SP    3  20    - - - -
            uint16_t address = BusRead(bus, cpu->pc);
            address |= BusRead(bus, cpu->pc + 1) << 8;

            cpu->pc += 2;

            BusWrite(bus, address, cpu->sp & 0xFF);
            BusWrite(bus, address + 1, (cpu->sp >> 8) & 0xFF);
            
            return 20;
        }
        case 0x88: { // ADC A, B    1  4     Z 0 H C
            uint16_t result = cpu->a + cpu->b + (cpu->f & 0x10 ? 1 : 0);
            uint8_t originA = cpu->a;
            uint8_t carryIn = (cpu->f & 0x10 ? 1 : 0);
            cpu->a = result & 0xFF;

            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;

            if ((((originA & 0x0F) + (cpu->b & 0x0F) + carryIn) & 0x10) != 0) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            if (result > 0xFF) {
                cpu->f |= 0x10;
            } else {
                cpu->f &= ~0x10;
            }

            return 4;
        }
        case 0x89: { // ADC A, C    1  4    Z 0 H C
            uint16_t result = cpu->a + cpu->c + (cpu->f & 0x10 ? 1 : 0);
            uint8_t originA = cpu->a;
            uint8_t carryIn = (cpu->f & 0x10 ? 1 : 0);
            cpu->a = result & 0xFF;

            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;

            if ((((originA & 0x0F) + (cpu->c & 0x0F) + carryIn) & 0x10) != 0) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            if (result > 0xFF) {
                cpu->f |= 0x10;
            } else {
                cpu->f &= ~0x10;
            }
            
            return 4;
        }
        case 0x0E: { // LD C, n8     2  8   - - - -
            uint8_t value = BusRead(bus, cpu->pc);
            cpu->pc++;
            
            cpu->c = value;

            return 8;
        }
        case 0xDD: { // TODO COME BACK LATER!!!!!!
            return 4;
        }
        case 0xD9: { // RETI    1  16   - - - - // TODO COME BACK LATER!!!!!!
            //* Interrupt
            uint16_t address = BusRead(bus, cpu->sp);
            address |= BusRead(bus, cpu->sp + 1) << 8;
            cpu->sp += 2;

            cpu->pc = address;

            return 16;
        }
        case 0xFF: { // RST $38    1 16    - - - - // ! CHECK
            cpu->sp -= 2;
            BusWrite(bus, cpu->sp, cpu->pc & 0xFF); //
            BusWrite(bus, cpu->sp + 1, (cpu->pc >> 8) & 0xFF);

            cpu->pc = 0x38;
            return 16;
        }
        case 0xAF: { // XOR A, A     1  4     1 0 0 0
            cpu->a = cpu->a ^ cpu->a; // 0
            
            cpu->f |= 0x80;

            cpu->f &= ~0x40;
            
            cpu->f &= ~0x20;

            cpu->f &= ~0x10;
            
            return 4;
        }
        case 0x21: { // LD HL, n16     3  12    - - - -
            cpu->hl = BusRead(bus, cpu->pc);
            cpu->hl |= BusRead(bus, cpu->pc + 1) << 8;
            cpu->pc += 2;
            
            return 12;
        }
        case 0x06: { // LD B, n8     2  8     - - - -
            uint8_t value = BusRead(bus, cpu->pc);
            cpu->b = value;

            cpu->pc += 1;
            return 8;
        }
        case 0x32: { // LD [HL-], A    1  8    - - - -
            uint16_t address = (cpu->h << 8) | cpu->l;
            BusWrite(bus, address, cpu->a);
            cpu->hl--;


            return 8;
        }
        case 0x05: { // DEC B    1  4    Z 1 H -
            uint8_t result = cpu->b - 1;
            uint8_t originB = cpu->b;
            cpu->b = result;

            if (cpu->b == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f |= 0x40;

            if ((((originB & 0x0F) - 1) & 0x10) != 0) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }
            
            return 4;
        }
        case 0x20: { // JR NZ, e8    2  12/8    - - - -
            int8_t offset = (int8_t)BusRead(bus, cpu->pc);
            cpu->pc += 1;

            if ((cpu->f & 0x80) == 0) {
                cpu->pc += offset;
                return 12;
            }
            return 8;
        }
        case 0x3E: { // LD A, n8    2  8   - - - -1
            uint8_t value = BusRead(bus, cpu->pc);
            cpu->a = value;
            cpu->pc += 1;

            return 8;
        }
        case 0xF3: { // DI   1  4   - - - -
            // TODO COME BACK LATER!!!!!!
            cpu->ime = 0;
            return 4;
        }
        case 0xE0: { // LDH [a8], A    2  12    - - - -
            uint8_t offset = BusRead(bus, cpu->pc);
            cpu->pc += 1;
            uint16_t address = 0xFF00 + offset;
            BusWrite(bus, address, cpu->a);

            printf("LDH [0x%02X], A: Wrote 0x%02X to 0x%04X\n", offset, cpu->a, address);

            return 12;
        }
        case 0xF0: { // LDH A, [a8]  2  12  - - - -
            uint8_t offset = BusRead(bus, cpu->pc);
            cpu->pc += 1;
            uint16_t address = 0xFF00 + offset;
            cpu->a = BusRead(bus, address);

            printf("LDH A, [0x%02X]: Read 0x%02X from 0x%04X\n", offset, cpu->a, address);

            return 12;
        }
        case 0xFE: { // CP A, n8   2  8   Z 1 H C
            uint8_t value = BusRead(bus, cpu->pc);
            cpu->pc += 1;

            uint16_t result = cpu->a - value;


            if ((result & 0xFF) == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            } 
            
            cpu->f |= 0x40;

            if ((((cpu->a & 0x0F) - (value & 0x0F)) & 0x10) != 0) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            if (result > 0xFF) {
                cpu->f |= 0x10;
            } else {
                cpu->f &= ~0x10;
            }
            
            return 8;
        }
        case 0x36: { // LD [HL], n8  2  12  - - - -
            uint8_t value = BusRead(bus, cpu->pc);
            cpu->pc += 1;
            uint16_t address = (cpu->h << 8) | cpu->l;
            BusWrite(bus, address, value);
            
            return 12;
        }
        case 0xEA: { // LD [a16], A  3  16  - - - -
            uint16_t address = BusRead(bus, cpu->pc);
            address |= BusRead(bus, cpu->pc + 1) << 8;
            cpu->pc += 2;

            BusWrite(bus, address, cpu->a);
            return 16;
        }
        case 0x31: { // JR NC, e8  2  12/8  - - - -
            int8_t offset = (int8_t)BusRead(bus, cpu->pc);
            cpu->pc += 1;
            
            if ((cpu->f & 0x10) == 0) {
                cpu->pc += offset;
                return 12;
            }
            return 8;
        }
        case 0x39: { // ADD HL, SP  1  8  - 0 H C
            uint16_t result = cpu->hl + cpu->sp;
            uint16_t originHL = cpu->hl;
            cpu->hl = result & 0xFFFF;

            cpu->f &= ~0x40;

            if ((((originHL & 0x0FFF) + (cpu->sp & 0x0FFF)) & 0x10) != 0) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            if (result > 0xFFFF) {
                cpu->f |= 0x10;
            } else {
                cpu->f &= ~0x10;
            }
            
            return 8;
        }
        case 0x28: { // JR Z, e8   2  12/8   - - - -
            
        }
        default:
            printf("Crash: opcode 0x%02X at pc 0x%04X\n", opcode, cpu->pc - 1);
            exit(1);
    }
}
