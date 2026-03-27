#include <cpu_prefix.h>

uint8_t get_cb_value(CPU *cpu, Bus *bus, uint8_t reg) {
    switch (reg) {
        case 0: return cpu->b;
        case 1: return cpu->c;
        case 2: return cpu->d;
        case 3: return cpu->e;
        case 4: return cpu->h;
        case 5: return cpu->l;
        case 6: return BusRead(bus, cpu->hl);
        case 7: return cpu->a;
        default: return 0;
    }
}

void set_cb_value(CPU *cpu, Bus *bus, uint8_t reg, uint8_t value) {
    switch (reg) {
        case 0: cpu->b = value; break;
        case 1: cpu->c = value; break;
        case 2: cpu->d = value; break;
        case 3: cpu->e = value; break;
        case 4: cpu->h = value; break;
        case 5: cpu->l = value; break;
        case 6: BusWrite(bus, cpu->hl, value); break;
        case 7: cpu->a = value; break;
    }
}