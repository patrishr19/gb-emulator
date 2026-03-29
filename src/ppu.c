#include <setup.h>
#include <emulator.h>
#include <bus.h>
#include <ppu.h>

static ppu_context ctx;

void ppu_oam_write(uint16_t address, uint8_t value) {
    if (address >= 0xFE00) {
        address -= 0xFE00;
    }

    uint8_t *p = (uint8_t *)ctx.oam_ram;
    p[address] = value;
}
uint8_t ppu_oam_read(uint16_t address) {
    if (address >= 0xFE00) {
        address -= 0xFE00;
    }

    uint8_t *p = (uint8_t *)ctx.oam_ram;
    return p[address];
}

void ppu_vram_write(uint16_t address, uint8_t value) {
    ctx.vram[address - 0x8000] = value;
}
uint8_t ppu_vram_read(uint16_t address) {
    return ctx.vram[address - 0x8000];
}