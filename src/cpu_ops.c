#include <setup.h>
#include <emulator.h>
#include <cpu.h>
#include <bus.h>
#include <iogm.h>

void op_xor(CPU *cpu, uint8_t value) {
    cpu->a ^= value;

    flagSet(cpu, FLAG_Z, cpu->a == 0);
    flagSet(cpu, FLAG_N, false);
    flagSet(cpu, FLAG_H, false);
    flagSet(cpu, FLAG_C, false); 
}

void op_and(CPU *cpu, uint8_t value) {
    cpu->a &= value;

    flagSet(cpu, FLAG_Z, cpu->a == 0);
    flagSet(cpu, FLAG_N, false);
    flagSet(cpu, FLAG_H, true);
    flagSet(cpu, FLAG_C, false);
}

void op_or(CPU *cpu, uint8_t value) {
    cpu->a |= value;

    flagSet(cpu, FLAG_Z, cpu->a == 0);
    flagSet(cpu, FLAG_N, false);
    flagSet(cpu, FLAG_H, false);
    flagSet(cpu, FLAG_C, false);
}

uint8_t op_inc_8(CPU *cpu, uint8_t value) {
    bool h = ((value & 0x0F) == 0x0F) ? true : false;
    value++;
    
    flagSet(cpu, FLAG_Z, value == 0);
    flagSet(cpu, FLAG_N, false);
    flagSet(cpu, FLAG_H, h);

    return value;
}

uint16_t op_inc_16(CPU *cpu, uint16_t value) {
    value++;
    return value;
}

uint8_t op_dec_8(CPU *cpu, uint8_t value) {
    bool h = ((value & 0x0F) == 0) ? true : false;
    value--;

    flagSet(cpu, FLAG_Z, value == 0);
    flagSet(cpu, FLAG_N, true);
    flagSet(cpu, FLAG_H, h);

    return value;
}

uint16_t op_dec_16(CPU *cpu, uint16_t value) {
    value--;
    return value;
}

void op_jr(CPU *cpu, Bus *bus) {
    int8_t offset = BusRead(bus, cpu->pc++);

    cpu->pc += offset;
}

void op_jp(CPU *cpu, Bus *bus) {
    uint16_t address = BusRead(bus, cpu->pc++);
    address |= BusRead(bus, cpu->pc++) << 8;

    cpu->pc = address;
}