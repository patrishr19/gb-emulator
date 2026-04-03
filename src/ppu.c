#include <setup.h>
#include <emulator.h>
#include <bus.h>
#include <ppu.h>
#include <ppu_sm.h>
#include <lcd.h>

static ppu_context ctx;

ppu_context *ppu_get_context() {
    return &ctx;
}

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

void ppu_init() {
    ctx.current_frame = 0;
    ctx.line_ticks = 0;
    ctx.video_buffer = malloc(YRES * XRES * sizeof(uint32_t));

    ctx.pfc.line_x = 0;
    ctx.pfc.pushed_x = 0;
    ctx.pfc.fetch_x = 0;
    ctx.pfc.pixel_fifo.size = 0;
    ctx.pfc.pixel_fifo.head = 0;
    ctx.pfc.cur_fetch_state = FS_TILE;

    lcd_init();
    LCDS_MODE_SET(MODE_OAM);

    memset(ctx.oam_ram, 0, sizeof(ctx.oam_ram));
    memset(ctx.video_buffer, 0, YRES * XRES * sizeof(uint32_t));
}

void ppu_tick(Bus *bus) {
    ctx.line_ticks += 1;

    switch(LCDS_MODE) {
        case MODE_OAM: ppu_mode_oam(); break;
        case MODE_XFER: ppu_mode_xfer(bus); break;
        case MODE_VBLANK: ppu_mode_vblank(bus); break;
        case MODE_HBLANK: ppu_mode_hblank(bus); break; 
    }
}


void request_interrupt(Bus *bus, uint8_t interruptBit) {
    uint8_t ifFlags = BusRead(bus, 0xFF0F);
    ifFlags |= interruptBit;
    BusWrite(bus, 0xFF0F, ifFlags);
}