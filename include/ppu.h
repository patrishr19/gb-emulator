#ifndef PPU_H
#define PPU_H

#include <setup.h>
#include <bus.h>

static const int LINES_PER_FRAME = 154;
static const int TICKS_PER_LINE = 456;
static const int YRES = 144;
static const int XRES = 160;


typedef enum {
    FS_TILE,
    FS_DATA0,
    FS_DATA1,
    FS_IDLE,
    FS_PUSH
} fetch_state;


typedef struct fifo_entry{
    struct fifo_entry *next;
    uint32_t value;
} fifo_entry;

typedef struct {
    fifo_entry *head;
    fifo_entry *tail;
    uint32_t  size;
} fifo;

typedef struct {
    fetch_state cur_fetch_state;
    fifo pixel_fifo;
    uint8_t line_x;
    uint8_t pushed_x;
    uint8_t fetch_x;
    uint8_t bgw_fetch_data[3];
    uint8_t fetch_entry_data[6];
    uint8_t map_y;
    uint8_t map_x;
    uint8_t tile_y;
    uint8_t fifo_x;
} pixel_fifo_context;

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

    pixel_fifo_context pfc;

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

void pipeline_process(Bus *bus);
void pipeline_fifo_reset();
#endif