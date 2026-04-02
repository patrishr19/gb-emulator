#include <setup.h>
#include <bus.h>
#include <dma.h>
#include <lcd.h>
#include <ppu.h>
static lcd_context ctx;

// colors
static unsigned long colors_default[4] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

void lcd_init() {
    ctx.lcdc = 0x91;
    ctx.scroll_x = 0;
    ctx.scroll_y = 0;
    ctx.ly = 0;
    ctx.ly_compare = 0;
    ctx.bg_palette = 0xFC;
    ctx.obj_palette[0] = 0xFF;
    ctx.obj_palette[1] = 0xFF;
    ctx.win_x = 0;
    ctx.win_y = 0;
    
    for (int i=0; i<4; i++) {
        ctx.bg_colors[i] = colors_default[i];
        ctx.sp1_colors[i] = colors_default[i];
        ctx.sp2_colors[i] = colors_default[i];
    }
}

lcd_context *lcd_get_context() {
    return &ctx;
}

uint8_t lcd_read(uint16_t address) {
    uint8_t offset = (address - 0xFF40);
    uint8_t *p = (uint8_t *)&ctx;

    return p[offset];
}

void update_palette(uint8_t palette_data, uint8_t pal) {
    uint32_t *p_colors = ctx.bg_colors;

    switch(pal) {
        case 1: p_colors = ctx.sp1_colors; break;
        case 2: p_colors = ctx.sp2_colors; break;
    }

    p_colors[0] = colors_default[palette_data & 0b11];
    p_colors[1] = colors_default[(palette_data >> 2) & 0b11];
    p_colors[2] = colors_default[(palette_data >> 4) & 0b11];
    p_colors[3] = colors_default[(palette_data >> 6) & 0b11];
}

void lcd_write(uint16_t address, uint8_t value) {
    uint8_t offset = (address - 0xFF40);
    uint8_t *p = (uint8_t *)&ctx;
    p[offset] = value;

    if (offset == 6) {
        //DMA
        dma_start(value);
    }

    if (address == 0xFF47) {
        update_palette(value, 0);
    } else if (address == 0xFF48) {
        update_palette(value & 0b11111100, 1);
    } else if (address == 0xFF49) {
        update_palette(value * 0b11111100, 2);
    }

}