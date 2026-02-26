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
    cpu->halt = 0;
}

int CPUStep(CPU *cpu, Bus *bus) {
    uint8_t ifFlags = BusRead(bus, 0xFF0F);
    uint8_t ieFlags = BusRead(bus, 0xFFFF);
    uint8_t interrupts = ifFlags & ieFlags;

    if (cpu->halt && interrupts) {
        cpu->halt = 0;
    }

    if (cpu->ime && interrupts) {
        if (interrupts & 0x01) {
            HandleInterrupt(cpu, bus, 0x40, 0x01);
            return 20;
        } else if (interrupts & 0x02) {
            HandleInterrupt(cpu, bus, 0x48, 0x02);
            return 20;
        } else if (interrupts & 0x04) {
            HandleInterrupt(cpu, bus, 0x50, 0x04);
            return 20;
        } else if (interrupts & 0x08) {
            HandleInterrupt(cpu, bus, 0x58, 0x08);
            return 20;
        } else if (interrupts & 0x10) {
            HandleInterrupt(cpu, bus, 0x60, 0x10);
            return 20;
        }
    }
    if (cpu->halt) {
        return 4;
    }
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
        case 0xCE: { // ADC A, n8   2 8    Z 0 H C
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
        case 0x66: { // LD H, [HL]  1  8  - - - -
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

            if ((originC & 0x0F) == 0) {
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
        case 0xD9: { // RETI    1  16   - - - - 
            //* Interrupt
            uint16_t address = BusRead(bus, cpu->sp);
            address |= BusRead(bus, cpu->sp + 1) << 8;
            cpu->sp += 2;

            cpu->pc = address;

            cpu->ime = 1;

            return 16;
        }
        case 0xFF: { // RST $38    1 16    - - - - // ! CHECK
            cpu->sp -= 2;
            BusWrite(bus, cpu->sp + 1, (cpu->pc >> 8) &  0xFF); //
            BusWrite(bus, cpu->sp, cpu->pc & 0xFF);

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
            cpu->l = BusRead(bus, cpu->pc++);
            cpu->h = BusRead(bus, cpu->pc++);
            
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

            if ((originB & 0x0F) == 0) {
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
        case 0x3E: { // LD A, n8    2  8   - - - -
            uint8_t value = BusRead(bus, cpu->pc);
            cpu->a = value;
            cpu->pc += 1;

            return 8;
        }
        case 0xF3: { // DI   1  4   - - - -
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

            if (cpu->a == value) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            } 
            
            cpu->f |= 0x40;

            if ((cpu->a & 0x0F) < (value & 0x0F)) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            if (cpu->a < value) {
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
        case 0x31: { //  LD SP, n16  3  12  - - - -
            uint16_t value = BusRead(bus, cpu->pc); 
            value |= (BusRead(bus, cpu->pc + 1) << 8);
            cpu->sp = value;
            cpu->pc += 2;
            return 12;
        }
        case 0x39: { // ADD HL, SP  1  8  - 0 H C
            uint16_t result = cpu->hl + cpu->sp;
            uint16_t originHL = cpu->hl;
            cpu->hl = result & 0xFFFF;

            cpu->f &= ~0x40;

            if ((((originHL & 0x0FFF) + (cpu->sp & 0x0FFF)) & 0x1000) != 0) {
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
        case 0x76: {
            cpu->halt = 1;
            return 4;
        }
        case 0xFB: {
            cpu->ime = 1;
            return 4;
        }
        case 0x2A: { // LD A, [HL+]   1  8   - - - -
            cpu->a = BusRead(bus, cpu->hl);
            cpu->hl++;
            return 8;
        }
        case 0xE2: { // LDH [C], A   1  8  - - - -
            uint16_t address = 0xFF00 + cpu->c;
            BusWrite(bus, address, cpu->a);
            return 8;
        }
        case 0xCD: { // CALL a16  3  24   - - - -
            uint16_t dest = BusRead(bus, cpu->pc); // 0000 1423
            dest |= (BusRead(bus, cpu->pc + 1) << 8);
            cpu->pc += 2;
            uint16_t returnAddress = cpu->pc;

            cpu->sp -= 2;
            BusWrite(bus, cpu->sp, returnAddress & 0xFF);
            BusWrite(bus, cpu->sp + 1, (returnAddress >> 8) & 0xFF);

            cpu->pc = dest;
            return 24;
        }
        case 0x01: { // LD BC, n16  3  12  - - - -
            cpu->c = BusRead(bus, cpu->pc++);
            cpu->b = BusRead(bus, cpu->pc++);
            
            return 12;
        }
        case 0xC9: { // RET  1  16  - - - -
            uint16_t address = BusRead(bus, cpu->sp);
            address |= BusRead(bus, cpu->sp + 1) << 8;
            cpu->sp += 2;

            cpu->pc = address;

            return 16;
        }
        case 0x78: { // LD A, B   1  4   - - - -
            cpu->a = cpu->b;
            return 4;
        }
        case 0xB1: { // OR A, C   1  4   Z 0 0 0
            cpu->a |= cpu-> c;
            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;

            cpu->f &= ~0x20;

            cpu->f &= ~0x10;

            return 4;
        }
        case 0xF5: { // PUSH AF  1  16  - - - -
            cpu->sp--;
            BusWrite(bus, cpu->sp, cpu->a);
            cpu->sp--;
            BusWrite(bus, cpu->sp, cpu->f);
            
            return 16;
        }
        case 0xC5: { // PUSH BC  1  16  - - - -
            cpu->sp--;
            BusWrite(bus, cpu->sp, cpu->b);
            cpu->sp--;
            BusWrite(bus, cpu->sp, cpu->c);

            return 16;
        }
        case 0xD5: { // PUSH DE  1  16  - - - -
            cpu->sp--;
            BusWrite(bus, cpu->sp, cpu->d);
            cpu->sp--;
            BusWrite(bus, cpu->sp, cpu->e);

            return 16;
        }
        case 0xE5: { // PUSH HL  1  16  - - - -
            cpu->sp--;
            BusWrite(bus, cpu->sp, cpu->h);
            cpu->sp--;
            BusWrite(bus, cpu->sp, cpu->l);

            return 16;
        }
        case 0xA7: { // AND A, A  1  4  Z 0 1 0
            cpu->a &= cpu->a;

            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;
            
            cpu->f |= 0x20;

            cpu->f &= ~0x10;

            return 4;
        }
        case 0x28: { // JR Z, e8  2  12/8  - - - -
            int8_t offset = (int8_t)BusRead(bus, cpu->pc);
            cpu->pc++;
            
            if (cpu->f & 0x80) {
                cpu->pc += offset;
                return 12;
            }
            return 8;
        }
        case 0xC0: {  // RET NZ  1  20/8  - - - -
            if (!(cpu->f & 0x80)) {
                uint16_t address = BusRead(bus, cpu->sp);
                address |= BusRead(bus, cpu->sp + 1) << 8;
                cpu->sp += 2;

                cpu->pc = address;
                return 20;
            }
            return 8;   
        }
        case 0xFA: { // LD A, [a16]  3  16  - - - -
            uint16_t address = BusRead(bus, cpu->pc);
            address |= (BusRead(bus, cpu->pc + 1) << 8);
            cpu->a = BusRead(bus, address);

            cpu->pc += 2;
            return 16;
        }
        case 0xC8: { // RET Z  1  20/8  - - - -
            if(cpu->f & 0x80) {
                uint16_t address = BusRead(bus, cpu->sp);
                address |= BusRead(bus, cpu->sp + 1) << 8;
                cpu->sp += 2;

                cpu->pc = address;
                return 20;
            }
            return 8;
        }
        case 0x3D: { // DEC A  1  4  Z 1 H -
            uint8_t originA = cpu->a;
            cpu->a -= 1;

            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f |= 0x40;

            if ((originA & 0x0F) == 0) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }


            return 4;
        }
        case 0x34: { // INC [HL]  1  12  Z 0 H -
            uint16_t address = (cpu->h << 8) | cpu->l;
            uint8_t value = BusRead(bus, address);

            // H flag
            if ((value & 0x0F) == 0x0F) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            value++;

            BusWrite(bus, address, value);

            if (value == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }
            
            cpu->f &= ~0x40;

            return 12;
        }
        case 0x3C: { // INC A  1  4   Z 0 H -
            if ((cpu->a & 0x0F) == 0x0F) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            cpu->a++;

            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;
            
            return 4;
        }
        case 0xE1: { // POP HL  1  12  - - - -
            cpu->l = BusRead(bus, cpu->sp++); // sp++ after busread
            cpu->h = BusRead(bus, cpu->sp++);

            return 12;
        }
        case 0xD1: { // POP DE  1  12  - - - -
            cpu->e = BusRead(bus, cpu->sp++);
            cpu->d = BusRead(bus, cpu->sp++);
            
            return 12;
        }
        case 0xC1: { // POP BC  1  12  - - - -
            cpu->c = BusRead(bus, cpu->sp++);
            cpu->b = BusRead(bus, cpu->sp++);
            
            return 12;
        }
        case 0xF1: { // POP AF  1  12  Z N H C
            cpu->f = BusRead(bus, cpu->sp++);
            cpu->a = BusRead(bus, cpu->sp++);

            cpu->f &= 0xF0;

            return 12;
        }
        case 0x2F: { // CPL  1  4  - 1 1 -
            cpu->a = ~cpu->a;

            cpu->f |= 0x40;
            cpu->f |= 0x20;

            return 4;

        }
        case 0x47: { // LD B, A  1 4  - - - -
            cpu->b = cpu->a;
            return 4;
        }
        case 0x11: { // LD DE, n16  3 12  - - - -
            cpu->e = BusRead(bus, cpu->pc++);
            cpu->d = BusRead(bus, cpu->pc++);
            
            return 12;
        }
        case 0x12: { // LD [DE], A  1 8  - - - -
            BusWrite(bus, cpu->de, cpu->a);
            return 8;
        }
        case 0x1C: { // INC E  1 4  Z 0 H -
            if ((cpu->e & 0x0F) == 0x0F) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }
            
            cpu->e++;

            if (cpu->e == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;

            return 4;

        }
        case 0x14: { // INC D  1 4  Z 0 H -
            if ((cpu->d & 0x0F) == 0x0F) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }
            
            cpu->d++;

            if (cpu->d == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;

            return 4;
        }
        case 0x7D: { // LD A, L  1 4  - - - -
            cpu->a = cpu->l;
            return 4;
        }
        case 0x7C: { // LD A, H  1 4  - - - -
            cpu->a = cpu->h;
            return 4;
        }
        case 0x18: { // JR e8  2 12  - - - -
            int8_t offset = (int8_t)BusRead(bus, cpu->pc++);
            cpu->pc += offset;

            return 12;
        }
        case 0x23: { // INC HL  1 8  - - - -
            cpu->hl++;
            return 8;
        }
        case 0xE6: { // AND A, n8  2 8  Z 0 1 0
            cpu->a &= BusRead(bus, cpu->pc++);

            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;
            
            cpu->f |= 0x20;

            cpu->f &= ~0x10;

            return 8;
        }
        case 0x10: { // STOP n8  2 4  - - - -
            cpu->pc++;
            // cpu->halt = 1;

            return 4;
        }
        case 0x45: { // LD B, L  1 4  - - - -
            cpu->b = cpu->l;
            return 4;
        }
        case 0xCA: { // JP Z, a16  3 16/12  - - - -
            uint16_t address = BusRead(bus, cpu->pc++);
            address |= BusRead(bus, cpu->pc++) << 8;
            if (cpu->f & 0x80) {
                cpu->pc = address;
                return 16;
            }
            return 12;
        }
        case 0xC4: { // CALL NZ, a16  3 24/12  - - - -
            uint16_t dest = BusRead(bus, cpu->pc++);
            dest |= (BusRead(bus, cpu->pc++) << 8);

            if (!(cpu->f & 0x80)) {
                cpu->sp -= 2;
                BusWrite(bus, cpu->sp, cpu->pc & 0xFF);
                BusWrite(bus, cpu->sp + 1, (cpu->pc >> 8) & 0xFF);

                cpu->pc = dest;
                return 24;
            }
            return 12;
        }
        case 0x77: { // LD [HL], A  1 8  - - - -
            uint16_t address = (cpu->h << 8) | cpu->l;
            BusWrite(bus, address, cpu->a);
            return 8;
        }
        case 0x2C: { // INC L  1 4  Z 0 H -
            if ((cpu->l & 0x0F) == 0x0F) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            cpu->l++;

            if (cpu->l == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;

            return 4;
        }
        case 0x24: { // INC H  1 4  Z 0 H -
            if ((cpu->h & 0x0F) == 0x0F) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            cpu->h++;

            if (cpu->h == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;

            return 4;
        }
        case 0x1A: { // LD A, [DE]  1 8  - - - -
            cpu->a = BusRead(bus, cpu->de);
            return 8;
        }
        case 0x13: { // INC DE  1 8  - - - -
            cpu->de++;
            return 8;
        }
        case 0xA9: { // XOR A, C  1 4  Z 0 0 0
            cpu->a = cpu->a ^ cpu->c;
            
            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;
            
            cpu->f &= ~0x20;

            cpu->f &= ~0x10;

            return 4;
        }
        case 0x22: { // LD [HL+], A  1 8  - - - -
            BusWrite(bus, cpu->hl, cpu->a);
            cpu->hl++;
            return 8;
        }
        case 0xC6: { // ADD A, n8  2 8  Z 0 H C
            uint8_t n8 = BusRead(bus, cpu->pc++);
            uint16_t result =  cpu->a + n8;

            if ((cpu->a & 0x0F) + (n8 & 0x0F) > 0x0F) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            if (result > 0xFF) {
                cpu->f |= 0x10;
            } else {
                cpu->f &= ~0x10;
            }

            cpu->a = (uint8_t)result;

            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;

            return 8;
        }
        case 0xD6: { // SUB A, n8  2 8  Z 1 H C
            uint8_t n8 = BusRead(bus, cpu->pc++);

            if ((n8 & 0x0F) > (cpu->a & 0x0F)) {
                cpu->f |= 0x20;
            }  else {
                cpu->f &= ~0x20;
            }

            if (n8 > cpu->a) {
                cpu->f |= 0x10;
            } else {
                cpu->f &= ~0x10;
            }

            cpu->a -= n8;

            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f |= 0x40;

            return 8;
        }
        case 0xB7: { // OR A, A  1 4  Z 0 0 0
            cpu->a |= cpu->a;

            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~0x40;
            cpu->f &= ~0x20;
            cpu->f &= ~0x10;

            return 4; 
        }
        case 0x46: { // LD B, [HL]  1  8  - - - -
            cpu->b = BusRead(bus, cpu->hl);
            return 8;
        }
        case 0x2D: { // DEC L  1 4  Z 1 H -
            if ((cpu->l & 0x0F) == 0) {
                cpu->f |= 0x20;
            } else {
                cpu->f &= ~0x20;
            }

            cpu->l--;

            if (cpu->l == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f |= 0x40;

            return 4;
        }
        case 0x4E: { // LD C, [HL]  1 8  - - - -
            cpu->c = BusRead(bus, cpu->hl);
            return 8;
        }
        case 0x56: { // LD D, [HL]  1 8  - - - -
            cpu->d = BusRead(bus, cpu->hl);
            return 8;
        }
        case 0xAE: { // XOR A, [HL]  1 8  Z 0 0 0
            cpu->a = cpu->a ^ BusRead(bus, cpu->hl);
            
            if (cpu->a == 0) {
                cpu->f |= 0x80;
            } else {
                cpu->f &= ~0x80;
            }

            cpu->f &= ~ 0x40;
            cpu->f &= ~ 0x20;
            cpu->f &= ~ 0x10;

            return 8;
        }
        case 0x26: { // LD H, n8  2 8  - - - -
            cpu->h = BusRead(bus, cpu->pc++);
            return 8;
        }
        case 0xCB: { //! PREFIX
            
        }
        default: {
            printf("Crash: opcode 0x%02X at pc 0x%04X\n", opcode, cpu->pc - 1);
            exit(1);
        }
    }

    // interrupts check
    return 0;
}

void HandleInterrupt(CPU *cpu, Bus *bus, uint16_t handlerAddress, uint8_t interruptBit) {
    cpu->ime = 0;

    cpu->sp -= 2;
    BusWrite(bus, cpu->sp, cpu->pc & 0xFF);
    BusWrite(bus, cpu->sp + 1, (cpu->pc >> 8) & 0xFF);

    cpu->pc = handlerAddress;

    uint8_t ifFlags = BusRead(bus, 0xFF0F);
    BusWrite(bus, 0xFF0F, ifFlags & ~interruptBit);
}
