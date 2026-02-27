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

uint8_t op_ld_immediate(CPU *cpu, Bus *bus) {
    return BusRead(bus, cpu->pc++);
}

uint8_t op_add(CPU *cpu, uint8_t a, uint8_t b, bool useCarry) {
    uint8_t cIn = useCarry && flagGet(cpu, FLAG_C) ? 1 : 0;
    uint16_t result = a + b + cIn;

    flagSet(cpu, FLAG_Z, (result & 0xFF) == 0);
    flagSet(cpu, FLAG_N, 0);
    flagSet(cpu, FLAG_H, (a & 0x0F) + (b & 0x0F) + cIn > 0x0F);
    flagSet(cpu, FLAG_C, result > 0xFF);

    return (uint8_t)result;
}

uint8_t op_sub(CPU *cpu, uint8_t a, uint8_t b, bool useCarry) {
    uint8_t cIn = useCarry && flagGet(cpu, FLAG_C) ? 1 : 0;

    flagSet(cpu, FLAG_Z, (a - b - cIn) == 0);
    flagSet(cpu, FLAG_N, 1);
    flagSet(cpu, FLAG_H, (a & 0x0F) < (b & 0x0F) + cIn);
    flagSet(cpu, FLAG_C, a < (uint16_t)b + cIn);

    return a - b - cIn;
}

void op_add_hl(CPU *cpu, uint16_t value) {
    uint32_t result = cpu->hl + value;

    flagSet(cpu, FLAG_N, 0);
    flagSet(cpu, FLAG_H, (cpu->hl & 0x0FFF) + (value & 0x0FFF) > 0x0FFF);
    flagSet(cpu, FLAG_C, result > 0xFFFF);

    cpu->hl = (uint16_t)result;
}

void op_call(CPU *cpu, Bus *bus) {
    uint16_t dest = BusRead16(bus, cpu->pc);
    cpu->pc += 2;

    BusWrite(bus, --cpu->sp, (cpu->pc >> 8) & 0xFF); // high
    BusWrite(bus, --cpu->sp, cpu->pc & 0xFF); // low

    cpu->pc = dest;
}

void op_ret(CPU *cpu, Bus *bus) {
    uint8_t low = BusRead(bus, cpu->sp++);
    uint8_t high = BusRead(bus, cpu->sp++);
    cpu->pc = (uint16_t)((high << 8) | low);
}

void op_rst(CPU *cpu, Bus *bus, uint16_t address) {
    BusWrite(bus, --cpu->sp, (cpu->pc >> 8) & 0xFF);
    BusWrite(bus, --cpu->sp, cpu->pc & 0xFF);

    cpu->pc = address;
}