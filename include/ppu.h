/**
 * @file ppu.h
 * @brief Main pixel processing unit
 * */

#pragma once

#include <setup.h>
#include <bus.h>

static const int LINES_PER_FRAME = 154;
static const int TICKS_PER_LINE = 456;
static const int YRES = 144;
static const int XRES = 160;

/**
 * @brief states for pixel fetcher
 * */
typedef enum {
    FS_TILE,
    FS_DATA0,
    FS_DATA1,
    FS_IDLE,
    FS_PUSH
} fetch_state;

/**
 * @brief Pixel node in the fifo queue
 * */
typedef struct fifo_entry{
    struct fifo_entry *next;
    uint32_t value;
} fifo_entry;
/**
 * @brief Queue struct managing pixels waiting to be pushed
 * */
typedef struct {
    fifo_entry *head;
    fifo_entry *tail;
    uint32_t  size;
} fifo;

/**
 * @brief Pipeline state context for fetching and pushing pixels
 * */
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


/**
 * @brief Object attribute memory layout for a sprite
 * */
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

/**
 * @brief Linked list tracking sprite structures
 * */
typedef struct _oam_line_entry {
    oam_entry entry;
    struct _oam_line_entry *next;
} oam_line_entry;


/**
 * @brief Global registry containging data spaces for ppu
 * */
typedef struct {
    oam_entry oam_ram[40];
    uint8_t vram[0x2000];

    uint8_t line_sprite_count; //0-10
    oam_line_entry *line_sprites; //linked list of current sprites on line
    oam_line_entry line_entry_array[10]; // mem to use for list

    uint8_t fetched_entry_count;
    oam_entry fetched_entries[3];
    uint8_t window_line;


    pixel_fifo_context pfc;

    uint32_t current_frame;
    uint32_t line_ticks;
    uint32_t *video_buffer;
} ppu_context;

void ppu_init();

/**
 * @brief Steps the PPU forward by single tick
 * */
void ppu_tick(Bus *bus);
/**
 * @brief Writing bytes into OAM 
 * */
void ppu_oam_write(uint16_t address, uint8_t value);

/**
 * @brief Reading bytes from OAM
 * */
uint8_t ppu_oam_read(uint16_t address);

/**
 * @brief Writing bytes to VRAM
 * */
void ppu_vram_write(uint16_t address, uint8_t value);
/**
 * @brief Reading bytes from VRAM
 * */
uint8_t ppu_vram_read(uint16_t address);

ppu_context *ppu_get_context();

/**
 * @brief Triggers a signal request flag inside the IF interrupt register
 * */
void request_interrupt(Bus *bus, uint8_t interruptBit);

/**
 * @brief Steps the clock cycle operations of the pixel fetcher
 * */
void pipeline_process(Bus *bus);

/**
 * @brief empties the fifo queue
 * */
void pipeline_fifo_reset();
