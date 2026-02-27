#include <setup.h>
#include <emulator.h>
#include <cpu.h>
#include <bus.h>
#include <iogm.h>
#include <cpu_ops.h>

void CPUInit(CPU *cpu) { // nintendo logo skip
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
        // NOP
        case 0x00: return 4;

        // XOR
        case 0xA8: op_xor(cpu, cpu->b); return 4;
        case 0xA9: op_xor(cpu, cpu->c); return 4;
        case 0xAA: op_xor(cpu, cpu->d); return 4;
        case 0xAB: op_xor(cpu, cpu->e); return 4;
        case 0xAC: op_xor(cpu, cpu->h); return 4;
        case 0xAD: op_xor(cpu, cpu->l); return 4;   
        case 0xAE: op_xor(cpu, BusRead(bus, cpu->hl)); return 8;
        case 0xAF: op_xor(cpu, cpu->a); return 4;
        case 0xEE: op_xor(cpu, BusRead(bus, cpu->pc++)); return 8;

        // AND
        case 0xA0: op_and(cpu, cpu->b); return 4;
        case 0xA1: op_and(cpu, cpu->c); return 4;
        case 0xA2: op_and(cpu, cpu->d); return 4;
        case 0xA3: op_and(cpu, cpu->e); return 4;
        case 0xA4: op_and(cpu, cpu->h); return 4;
        case 0xA5: op_and(cpu, cpu->l); return 4;
        case 0xA6: op_and(cpu, BusRead(bus, cpu->hl)); return 8;
        case 0xA7: op_and(cpu, cpu->a); return 4;
        case 0xE6: op_and(cpu, BusRead(bus, cpu->pc++)); return 8;

        // OR
        case 0xB0: op_or(cpu, cpu->b); return 4;
        case 0xB1: op_or(cpu, cpu->c); return 4;
        case 0xB2: op_or(cpu, cpu->d); return 4;
        case 0xB3: op_or(cpu, cpu->e); return 4;
        case 0xB4: op_or(cpu, cpu->h); return 4;
        case 0xB5: op_or(cpu, cpu->l); return 4;
        case 0xB6: op_or(cpu, BusRead(bus, cpu->hl)); return 8;
        case 0xB7: op_or(cpu, cpu->a); return 4;
        case 0xF6: op_or(cpu, BusRead(bus, cpu->pc++)); return 8;

        // INC 8
        case 0x04: cpu->b = op_inc_8(cpu, cpu->b); return 4;
        case 0x14: cpu->d = op_inc_8(cpu, cpu->d); return 4;
        case 0x24: cpu->h = op_inc_8(cpu, cpu->h); return 4;
        case 0x34: {
            uint8_t value = BusRead(bus, cpu->hl);
            value = op_inc_8(cpu, value);
            BusWrite(bus, cpu->hl, value);
            return 12; }
        case 0x0C: cpu->c = op_inc_8(cpu, cpu->c); return 4;
        case 0x1C: cpu->e = op_inc_8(cpu, cpu->e); return 4;
        case 0x2C: cpu->l = op_inc_8(cpu, cpu->l); return 4;
        case 0x3C: cpu->a = op_inc_8(cpu, cpu->a); return 4;

        // INC 16
        case 0x03: cpu->bc = op_inc_16(cpu, cpu->bc); return 8;
        case 0x13: cpu->de = op_inc_16(cpu, cpu->de); return 8;
        case 0x23: cpu->hl = op_inc_16(cpu, cpu->hl); return 8;
        case 0x33: cpu->sp = op_inc_16(cpu, cpu->sp); return 8;

        // DEC 8
        case 0x05: cpu->b = op_dec_8(cpu, cpu->b); return 4;
        case 0x15: cpu->d = op_dec_8(cpu, cpu->d); return 4;
        case 0x25: cpu->h = op_dec_8(cpu, cpu->h); return 4;
        case 0x35: {
            uint8_t value = BusRead(bus, cpu->hl);
            value = op_dec_8(cpu, value);
            BusWrite(bus, cpu->hl, value);
            return 12; }
        case 0x0D: cpu->c = op_dec_8(cpu, cpu->c); return 4;
        case 0x1D: cpu->e = op_dec_8(cpu, cpu->e); return 4;
        case 0x2D: cpu->l = op_dec_8(cpu, cpu->l); return 4;
        case 0x3D: cpu->a = op_dec_8(cpu, cpu->a); return 4;

        // DEC 16
        case 0x0B: cpu->bc = op_dec_16(cpu, cpu->bc); return 8;
        case 0x1B: cpu->de = op_dec_16(cpu, cpu->de); return 8;
        case 0x2B: cpu->hl = op_dec_16(cpu, cpu->hl); return 8;
        case 0x3B: cpu->sp = op_dec_16(cpu, cpu->sp); return 8;

        // JR
        case 0x18: op_jr(cpu, bus); return 12;
        case 0x20:
            if (!(flagGet(cpu, FLAG_Z))) {
                op_jr(cpu, bus);
                return 12;
            } else {
                cpu->pc++;
                return 8;
            }
        case 0x28:
            if (flagGet(cpu, FLAG_Z)) {
                op_jr(cpu, bus);
                return 12;
            } else {
                cpu->pc++;
                return 8;
            }
        case 0x30:
            if (!(flagGet(cpu, FLAG_C))) { 
                op_jr(cpu, bus);
                return 12;
            } else {
                cpu->pc++;
                return 8;
            }
        case 0x38:
            if (flagGet(cpu, FLAG_C)) {
                op_jr(cpu, bus);
                return 12;
            } else {
                cpu->pc++;
                return 8;
            }

        // JP
        case 0xC2:
            if (!(flagGet(cpu, FLAG_Z))) {
                op_jp(cpu, bus);
                return 16;
            } else {
                cpu->pc += 2;
                return 12;
            }
        case 0xD2:
            if (!(flagGet(cpu, FLAG_C))) {
                op_jp(cpu, bus);
                return 16;
            } else {
                cpu->pc += 2;
                return 12;
            }
        case 0xC3: op_jp(cpu, bus); return 16;
        case 0xCA:
            if (flagGet(cpu, FLAG_Z)) {
                op_jp(cpu, bus);
                return 16;
            } else {
                cpu->pc += 2;
                return 12;
            }
        case 0xDA:
            if (flagGet(cpu, FLAG_C)) {
                op_jp(cpu, bus);
                return 16;
            } else {
                cpu->pc += 2;
                return 12;
            }
        case 0xE9: cpu->pc = cpu->hl; return 4;
    
        // LD Immediate
        case 0x06: cpu->b = op_ld_immediate(cpu, bus); return 8;
        case 0x16: cpu->d = op_ld_immediate(cpu, bus); return 8;
        case 0x26: cpu->h = op_ld_immediate(cpu, bus); return 8;
        case 0x36:
            BusWrite(bus, cpu->hl, op_ld_immediate(cpu, bus));
            return 12;
        case 0x0E: cpu->c = op_ld_immediate(cpu, bus); return 8;
        case 0x1E: cpu->e = op_ld_immediate(cpu, bus); return 8;
        case 0x2E: cpu->l = op_ld_immediate(cpu, bus); return 8;
        case 0x3E: cpu->a = op_ld_immediate(cpu, bus); return 8;
        
        // LD Register to register + halt
        case 0x40: cpu->b = cpu->b; return 4;
        case 0x41: cpu->b = cpu->c; return 4;
        case 0x42: cpu->b = cpu->d; return 4;
        case 0x43: cpu->b = cpu->e; return 4;
        case 0x44: cpu->b = cpu->h; return 4;
        case 0x45: cpu->b = cpu->l; return 4;
        case 0x46: cpu->b = BusRead(bus, cpu->hl); return 8;
        case 0x47: cpu->b = cpu->a; return 4;
        case 0x48: cpu->c = cpu->b; return 4;
        case 0x49: cpu->c = cpu->c; return 4;
        case 0x4A: cpu->c = cpu->d; return 4;
        case 0x4B: cpu->c = cpu->e; return 4;
        case 0x4C: cpu->c = cpu->h; return 4;
        case 0x4D: cpu->c = cpu->l; return 4;
        case 0x4E: cpu->c = BusRead(bus, cpu->hl); return 8;
        case 0x4F: cpu->c = cpu->a; return 4;

        case 0x50: cpu->d = cpu->b; return 4;
        case 0x51: cpu->d = cpu->c; return 4;
        case 0x52: cpu->d = cpu->d; return 4;
        case 0x53: cpu->d = cpu->e; return 4;
        case 0x54: cpu->d = cpu->h; return 4;
        case 0x55: cpu->d = cpu->l; return 4;
        case 0x56: cpu->d = BusRead(bus, cpu->hl); return 8;
        case 0x57: cpu->d = cpu->a; return 4;
        case 0x58: cpu->e = cpu->b; return 4;
        case 0x59: cpu->e = cpu->c; return 4;
        case 0x5A: cpu->e = cpu->d; return 4;
        case 0x5B: cpu->e = cpu->e; return 4;
        case 0x5C: cpu->e = cpu->h; return 4;
        case 0x5D: cpu->e = cpu->l; return 4;
        case 0x5E: cpu->e = BusRead(bus, cpu->hl); return 8;
        case 0x5F: cpu->e = cpu->a; return 4;

        case 0x60: cpu->h = cpu->b; return 4;
        case 0x61: cpu->h = cpu->c; return 4;
        case 0x62: cpu->h = cpu->d; return 4;
        case 0x63: cpu->h = cpu->e; return 4;
        case 0x64: cpu->h = cpu->h; return 4;
        case 0x65: cpu->h = cpu->l; return 4;
        case 0x66: cpu->h = BusRead(bus, cpu->hl); return 8;
        case 0x67: cpu->h = cpu->a; return 4;
        case 0x68: cpu->l = cpu->b; return 4;
        case 0x69: cpu->l = cpu->c; return 4;
        case 0x6A: cpu->l = cpu->d; return 4;
        case 0x6B: cpu->l = cpu->e; return 4;
        case 0x6C: cpu->l = cpu->h; return 4;
        case 0x6D: cpu->l = cpu->l; return 4;
        case 0x6E: cpu->l = BusRead(bus, cpu->hl); return 8;
        case 0x6F: cpu->l = cpu->a; return 4;

        case 0x70: BusWrite(bus, cpu->hl, cpu->b); return 8;
        case 0x71: BusWrite(bus, cpu->hl, cpu->c); return 8;
        case 0x72: BusWrite(bus, cpu->hl, cpu->d); return 8;
        case 0x73: BusWrite(bus, cpu->hl, cpu->e); return 8;
        case 0x74: BusWrite(bus, cpu->hl, cpu->h); return 8;
        case 0x75: BusWrite(bus, cpu->hl, cpu->l); return 8;
        case 0x76: cpu->halt = 1; return 4;
        case 0x77: BusWrite(bus, cpu->hl, cpu->a); return 8;
        case 0x78: cpu->a = cpu->b; return 4;
        case 0x79: cpu->a = cpu->c; return 4;
        case 0x7A: cpu->a = cpu->d; return 4;
        case 0x7B: cpu->a = cpu->e; return 4;
        case 0x7C: cpu->a = cpu->h; return 4;
        case 0x7D: cpu->a = cpu->l; return 4;
        case 0x7E: cpu->a = BusRead(bus, cpu->hl); return 8;
        case 0x7F: cpu->a = cpu->a; return 4;
        
        // LD High ram
        case 0xE0: {
            uint8_t offset = BusRead(bus, cpu->pc++);
            BusWrite(bus, 0xFF00 + offset, cpu->a);
            return 12; }
        case 0xF0: {
            uint8_t offset = BusRead(bus, cpu->pc++);
            cpu->a = BusRead(bus, 0xFF00 + offset);
            return 12; }

        // LD Absolute
        case 0xEA: {
            uint16_t address = BusRead16(bus, cpu->pc++);
            cpu->pc += 2;
            BusWrite(bus, address, cpu->a);
            return 16; }
        case 0xFA: {
            uint16_t address = BusRead16(bus, cpu->pc);
            cpu->pc += 2;
            cpu->a = BusRead(bus, address);
            return 16; }
            
        // LD C offset
        case 0xE2:
            BusWrite(bus, 0xFF00 + cpu->c, cpu->a);
            return 8;
        case 0xF2:
            cpu->a = BusRead(bus, 0xFF00 + cpu->c);
            return 8;
        
        // LD accumulator loads
        case 0x02: BusWrite(bus, cpu->bc, cpu->a); return 8;
        case 0x12: BusWrite(bus, cpu->de, cpu->a); return 8;
        case 0x22: BusWrite(bus, cpu->hl++, cpu->a); return 8;
        case 0x32: BusWrite(bus, cpu->hl--, cpu->a); return 8;
        case 0x0A: cpu->a = BusRead(bus, cpu->bc); return 8;
        case 0x1A: cpu->a = BusRead(bus, cpu->de); return 8;
        case 0x2A: cpu->a = BusRead(bus, cpu->hl++); return 8;
        case 0x3A: cpu->a = BusRead(bus, cpu->hl--); return 8;

        // LD Immediate 16
        case 0x01:
            cpu->bc = BusRead16(bus, cpu->pc);
            cpu->pc += 2;
            return 12;
        case 0x11:
            cpu->de = BusRead16(bus, cpu->pc);
            cpu->pc += 2;
            return 12;
        case 0x21:
            cpu->hl = BusRead16(bus, cpu->pc);
            cpu->pc += 2;
            return 12;
        case 0x31:
            cpu->sp = BusRead16(bus, cpu->pc);
            cpu->pc += 2;
            return 12;

        // LD stack pointer
        case 0x08: {
            uint16_t address = BusRead16(bus, cpu->pc);
            cpu->pc += 2;
            BusWrite(bus, address, cpu->sp & 0xFF);
            BusWrite(bus, address + 1, (cpu->sp >> 8) & 0xFF);
            return 20; }
        case 0xF8: {
            int8_t offset = (int8_t)BusRead(bus, cpu->pc++);
            bool h = ((cpu->sp & 0x0F) + (offset & 0x0F)) > 0xF;
            bool c = ((cpu->sp & 0xFF) + (offset & 0xFF)) > 0xFF;

            cpu->hl = cpu->sp + offset;

            flagSet(cpu, FLAG_Z, 0);
            flagSet(cpu, FLAG_N, 0);
            flagSet(cpu, FLAG_H, h);
            flagSet(cpu, FLAG_C, c);

            return 12; }
        case 0xF9: cpu->sp = cpu->hl; return 8;
        
        // ADD
        case 0x80: cpu->a = op_add(cpu, cpu->a, cpu->b, false); return 4;
        case 0x81: cpu->a = op_add(cpu, cpu->a, cpu->c, false); return 4;
        case 0x82: cpu->a = op_add(cpu, cpu->a, cpu->d, false); return 4;
        case 0x83: cpu->a = op_add(cpu, cpu->a, cpu->e, false); return 4;
        case 0x84: cpu->a = op_add(cpu, cpu->a, cpu->h, false); return 4;
        case 0x85: cpu->a = op_add(cpu, cpu->a, cpu->l, false); return 4;
        case 0x86: cpu->a = op_add(cpu, cpu->a, BusRead(bus, cpu->hl), false); return 8;
        case 0x87: cpu->a = op_add(cpu, cpu->a, cpu->a, false); return 4;
        case 0xC6: cpu->a = op_add(cpu, cpu->a, BusRead(bus, cpu->pc++), false); return 8;
        
        // ADC
        case 0x88: cpu->a = op_add(cpu, cpu->a, cpu->b, true); return 4;
        case 0x89: cpu->a = op_add(cpu, cpu->a, cpu->c, true); return 4;
        case 0x8A: cpu->a = op_add(cpu, cpu->a, cpu->d, true); return 4;
        case 0x8B: cpu->a = op_add(cpu, cpu->a, cpu->e, true); return 4;
        case 0x8C: cpu->a = op_add(cpu, cpu->a, cpu->h, true); return 4;
        case 0x8D: cpu->a = op_add(cpu, cpu->a, cpu->l, true); return 4;
        case 0x8E: cpu->a = op_add(cpu, cpu->a, BusRead(bus, cpu->hl), true); return 8;
        case 0x8F: cpu->a = op_add(cpu, cpu->a, cpu->a, true); return 4;
        case 0xCE: cpu->a = op_add(cpu, cpu->a, BusRead(bus, cpu->pc++), true); return 8;
        
        // SUB
        case 0x90: cpu->a = op_sub(cpu, cpu->a, cpu->b, false); return 4;
        case 0x91: cpu->a = op_sub(cpu, cpu->a, cpu->c, false); return 4;
        case 0x92: cpu->a = op_sub(cpu, cpu->a, cpu->d, false); return 4;
        case 0x93: cpu->a = op_sub(cpu, cpu->a, cpu->e, false); return 4;
        case 0x94: cpu->a = op_sub(cpu, cpu->a, cpu->h, false); return 4;
        case 0x95: cpu->a = op_sub(cpu, cpu->a, cpu->l, false); return 4;
        case 0x96: cpu->a = op_sub(cpu, cpu->a, BusRead(bus, cpu->hl), false); return 8;
        case 0x97: cpu->a = op_sub(cpu, cpu->a, cpu->a, false); return 4;
        case 0xD6: cpu->a = op_sub(cpu, cpu->a, BusRead(bus, cpu->pc++), false); return 8;

        // SBC
        case 0x98: cpu->a = op_sub(cpu, cpu->a, cpu->b, true); return 4;
        case 0x99: cpu->a = op_sub(cpu, cpu->a, cpu->c, true); return 4;
        case 0x9A: cpu->a = op_sub(cpu, cpu->a, cpu->d, true); return 4;
        case 0x9B: cpu->a = op_sub(cpu, cpu->a, cpu->e, true); return 4;
        case 0x9C: cpu->a = op_sub(cpu, cpu->a, cpu->h, true); return 4;
        case 0x9D: cpu->a = op_sub(cpu, cpu->a, cpu->l, true); return 4;
        case 0x9E: cpu->a = op_sub(cpu, cpu->a, BusRead(bus, cpu->hl), true); return 8;
        case 0x9F: cpu->a = op_sub(cpu, cpu->a, cpu->a, true); return 4;
        case 0xDE: cpu->a = op_sub(cpu, cpu->a, BusRead(bus, cpu->pc++), true); return 8;
        
        // CP
        case 0xB8: (void)op_sub(cpu, cpu->a, cpu->b, false); return 4;
        case 0xB9: (void)op_sub(cpu, cpu->a, cpu->c, false); return 4;
        case 0xBA: (void)op_sub(cpu, cpu->a, cpu->d, false); return 4;
        case 0xBB: (void)op_sub(cpu, cpu->a, cpu->e, false); return 4;
        case 0xBC: (void)op_sub(cpu, cpu->a, cpu->h, false); return 4;
        case 0xBD: (void)op_sub(cpu, cpu->a, cpu->l, false); return 4;
        case 0xBE: (void)op_sub(cpu, cpu->a, BusRead(bus, cpu->hl), false); return 8;
        case 0xBF: (void)op_sub(cpu, cpu->a, cpu->a, false); return 4;
        case 0xFE: (void)op_sub(cpu, cpu->a, BusRead(bus, cpu->pc++), false); return 8;

        // ADD HL
        case 0x09: op_add_hl(cpu, cpu->bc); return 8;
        case 0x19: op_add_hl(cpu, cpu->de); return 8;
        case 0x29: op_add_hl(cpu, cpu->hl); return 8;
        case 0x39: op_add_hl(cpu, cpu->sp); return 8;
        
        // ADD SP
        case 0xE8: {
            int8_t offset = (int8_t)BusRead(bus, cpu->pc++);

            flagSet(cpu, FLAG_Z, 0);
            flagSet(cpu, FLAG_N, 0);
            flagSet(cpu, FLAG_H, (cpu->sp & 0x0F) + (offset & 0x0F) > 0x0F);
            flagSet(cpu, FLAG_C, (cpu->sp & 0xFF) + (offset & 0xFF) > 0xFF);

            cpu->sp += offset;
            return 16; }

        // POP
        case 0xC1:
            cpu->c = BusRead(bus, cpu->sp++);
            cpu->b = BusRead(bus, cpu->sp++);
            return 12;
        case 0xD1:
            cpu->e = BusRead(bus, cpu->sp++);
            cpu->d = BusRead(bus, cpu->sp++);
            return 12;
        case 0xE1:
            cpu->l = BusRead(bus, cpu->sp++);
            cpu->h = BusRead(bus, cpu->sp++);
            return 12;
        case 0xF1:
            cpu->f = BusRead(bus, cpu->sp++);
            cpu->a = BusRead(bus, cpu->sp++);
            cpu->f &= 0xF0;
            return 12;
        
        // PUSH
        case 0xC5:
            BusWrite(bus, --cpu->sp, cpu->b);
            BusWrite(bus, --cpu->sp, cpu->c);
            return 16;
        case 0xD5:
            BusWrite(bus, --cpu->sp, cpu->d);
            BusWrite(bus, --cpu->sp, cpu->e);
            return 16;
        case 0xE5:
            BusWrite(bus, --cpu->sp, cpu->h);
            BusWrite(bus, --cpu->sp, cpu->l);
            return 16;
        case 0xF5:
            BusWrite(bus, --cpu->sp, cpu->a);
            BusWrite(bus, --cpu->sp, cpu->f);
            return 16;

        // CALL
        case 0xC4:
            if (!flagGet(cpu, FLAG_Z)) {
                op_call(cpu, bus);
                return 24;
            } else {
                cpu->pc += 2;
                return 12;
            }
        case 0xD4:
            if (!flagGet(cpu, FLAG_C)) {
                op_call(cpu, bus);
                return 24;
            } else {
                cpu->pc += 2;
                return 12;
            }
        case 0xCC:
            if(flagGet(cpu, FLAG_Z)) {
                op_call(cpu, bus);
                return 24;
            } else {
                cpu->pc += 2;
                return 12;
            }
        case 0xCD:
            op_call(cpu, bus); return 24;
        case 0xDC:
            if (flagGet(cpu, FLAG_C)) {
                op_call(cpu,bus);
                return 24;
            } else {
                cpu->pc += 2;
                return 12;
            }

        // RET
        case 0xC0:
            if (!flagGet(cpu, FLAG_Z)) {
                op_ret(cpu, bus);
                return 20;
            } else {
                return 8;
            }
        case 0xD0:
            if (!flagGet(cpu, FLAG_C)) {
                op_ret(cpu, bus);
                return 20;
            } else {
                return 8;
            }
        case 0xC8:
            if (flagGet(cpu, FLAG_Z)) {
                op_ret(cpu, bus);
                return 20;
            } else {
                return 8;
            }
        case 0xC9: op_ret(cpu, bus); return 16;
        case 0xD8:
            if (flagGet(cpu, FLAG_C)) {
                op_ret(cpu, bus);
                return 20;
            } else {
                return 8;
            }

        // RETI
        case 0xD9: op_ret(cpu, bus); cpu->ime = 1; return 16;
        
        // RST
        case 0xC7: op_rst(cpu, bus, 0x0000); return 16;
        case 0xD7: op_rst(cpu, bus, 0x0010); return 16;
        case 0xE7: op_rst(cpu, bus, 0x0020); return 16;
        case 0xF7: op_rst(cpu, bus, 0x0030); return 16;
        case 0xCF: op_rst(cpu, bus, 0x0008); return 16;
        case 0xDF: op_rst(cpu, bus, 0x0018); return 16;
        case 0xEF: op_rst(cpu, bus, 0x0028); return 16;
        case 0xFF: op_rst(cpu, bus, 0x0038); return 16;
        
        // SYSTEM
        case 0xF3: cpu->ime = 0; return 4;
        case 0xFB: cpu->ime = 1; return 4; // TODO Scheduling
        case 0x10: cpu->pc++; cpu->halt = 1; return 4;

        // ROTATION
        case 0x0F: { // RRCA
            uint8_t bit0 = cpu->a & 0x01;

            cpu->a >>= 1;
            cpu->a |= (bit0 << 7);
            flagSet(cpu, FLAG_Z, 0);
            flagSet(cpu, FLAG_N, 0);
            flagSet(cpu, FLAG_H, 0);
            flagSet(cpu, FLAG_C, bit0);

            return 4; }
        case 0x07: { // RLCA
            uint8_t bit7 = (cpu->a & 0x80) >> 7; // first place

            cpu->a <<=  1;
            cpu->a |= bit7;

            flagSet(cpu, FLAG_Z, 0);
            flagSet(cpu, FLAG_N, 0);
            flagSet(cpu, FLAG_H, 0);
            flagSet(cpu, FLAG_C, bit7);
            return 4; }
        case 0x17: { // RLA
            uint8_t oldCarry = flagGet(cpu, FLAG_C) ? 1 : 0;
            uint8_t bit7 = (cpu->a & 0x80) >> 7; 
            cpu->a = (cpu->a << 1) | oldCarry;

            flagSet(cpu, FLAG_Z, 0);
            flagSet(cpu, FLAG_N, 0);
            flagSet(cpu, FLAG_H, 0);
            flagSet(cpu, FLAG_C, bit7);
            return 4; }
        case 0x1F: { // RRA
            uint8_t oldCarry = flagGet(cpu, FLAG_C) ? 1 : 0;
            uint8_t bit0 = cpu->a & 0x01;
            cpu->a = (cpu->a >> 1) | (oldCarry << 7);

            flagSet(cpu, FLAG_Z, 0);
            flagSet(cpu, FLAG_N, 0);
            flagSet(cpu, FLAG_H, 0);
            flagSet(cpu, FLAG_C, bit0);
            return 4;
        }

        // Acumulator manipulation
        case 0x27: { // DAA 
            uint8_t corr = 0;
            bool setCarry = false;

            if (flagGet(cpu, FLAG_H) || (!flagGet(cpu, FLAG_N) && (cpu->a & 0x0F) > 9)) {
                corr |= 0x06;
            }

            if (flagGet(cpu, FLAG_C) || (!flagGet(cpu, FLAG_N) && cpu->a > 0x99)) {
                corr |= 0x60;
                setCarry = true;
            }

            if (flagGet(cpu, FLAG_N)) {
                cpu->a -= corr;
            } else {
                cpu->a += corr;
            }

            flagSet(cpu, FLAG_Z, cpu->a == 0);
            flagSet(cpu, FLAG_H, 0);
            flagSet(cpu, FLAG_C, setCarry);

            return 4; }
        case 0x37: // SCF
            flagSet(cpu, FLAG_N, 0);
            flagSet(cpu, FLAG_H, 0);
            flagSet(cpu, FLAG_C, 1);
            return 4;
        case 0x2F: // CPL
            cpu->a = ~cpu->a;
            flagSet(cpu, FLAG_N, 1);
            flagSet(cpu, FLAG_H, 1);
            return 4;
        case 0x3F: // CCF
            flagSet(cpu, FLAG_N, 0);
            flagSet(cpu, FLAG_H, 0);
            flagSet(cpu, FLAG_C, !flagGet(cpu, FLAG_C));
            return 4;          
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

void flagSet(CPU *cpu, uint8_t flag, bool value) {
    if (value) {
        cpu->f |= flag;
    } else {
        cpu->f &= ~flag;
    }
}

bool flagGet(CPU *cpu, uint8_t flag) {
    return (cpu->f & flag) != 0;
}