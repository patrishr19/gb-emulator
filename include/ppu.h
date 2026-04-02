#ifndef PPU_H
#define PPU_H

#include <setup.h>

static const int LINES_PER_FRAME = 154;
static const int TICKS_PER_LINE = 456;
static const int YRES = 144;
static const int XRES = 160;


typedef struct {
    uint8_t y;
    uint8_t x;
    uint8_t tile;
    
    unsigned f_cgb_pn : 3;
    unsigned f_cgb_vram_bank : 1;
    unsigned f_pn : 1;
    unsigned f_x_flip : 1;
    unsigned f_y_flip : 1;
    unsigned f_bgp : 1;

} oam_entry;

typedef struct {
    oam_entry oam_ram[40];
    uint8_t vram[0x2000];

    uint32_t current_frame;
    uint32_t line_ticks;
    uint32_t *video_buffer;
} ppu_context;

void ppu_init();
void ppu_tick(Bus *bus);

void ppu_oam_write(uint16_t address, uint8_t value);
uint8_t ppu_oam_read(uint16_t address);

void ppu_vram_write(uint16_t address, uint8_t value);
uint8_t ppu_vram_read(uint16_t address);

ppu_context *ppu_get_context();

void request_interrupt(Bus *bus, uint8_t interruptBit);
#endif