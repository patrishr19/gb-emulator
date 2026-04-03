#include <ppu.h>
#include <bus.h>
#include <cpu.h>
#include <setup.h>
#include <lcd.h>



void pixel_fifo_push(uint32_t value) {
    fifo_entry *next = malloc(sizeof(fifo_entry));
    next->next = NULL;
    next->value = value;

    if (!ppu_get_context()->pfc.pixel_fifo.head) {
        //first entry
        ppu_get_context()->pfc.pixel_fifo.head = ppu_get_context()->pfc.pixel_fifo.tail = next;
    } else {
        ppu_get_context()->pfc.pixel_fifo.tail->next = next;
        ppu_get_context()->pfc.pixel_fifo.tail = next;
    }

    ppu_get_context()->pfc.pixel_fifo.size++;
}

uint32_t pixel_fifo_pop() {
    if (ppu_get_context()->pfc.pixel_fifo.size <= 0) {
        fprintf(stderr, "ERROR in pixel fifo\n");
        exit(-8);
    }

    fifo_entry *popped = ppu_get_context()->pfc.pixel_fifo.head;
    ppu_get_context()->pfc.pixel_fifo.head = popped->next;
    ppu_get_context()->pfc.pixel_fifo.size--;

    uint32_t value = popped->value;
    free(popped);

    return value;
}

bool pipeline_fifo_add() {
    if (ppu_get_context()->pfc.pixel_fifo.size > 8) {
        //full
        return false;
    }

    int x = ppu_get_context()->pfc.fetch_x - (8 - (lcd_get_context()->scroll_x % 8));

    for (int i=0; i<8; i++) {
        int bit = 7 - i;
        uint8_t low = !!(ppu_get_context()->pfc.bgw_fetch_data[1] & (1 << bit));
        uint8_t high = !!(ppu_get_context()->pfc.bgw_fetch_data[2] & (1 << bit)) << 1;
        uint32_t color = lcd_get_context()->bg_colors[high | low];

        if (x >= 0) {
            pixel_fifo_push(color);
            ppu_get_context()->pfc.fifo_x++;
        }
    }

    return true;
}

void pipeline_fetch(Bus *bus) {
    switch(ppu_get_context()->pfc.cur_fetch_state) {
        case FS_TILE: {
            if (LCDC_BGW_ENABLE) {
                ppu_get_context()->pfc.bgw_fetch_data[0] = BusRead(bus, (LCDC_BG_MAP_AREA + 
                (ppu_get_context()->pfc.map_x / 8) + 
                ((ppu_get_context()->pfc.map_y / 8) * 32)));
                
                if (LCDC_BGW_DATA_AREA == 0x8800) {
                    ppu_get_context()->pfc.bgw_fetch_data[0] += 128;
                    // int8_t signed_int = (int8_t)ppu_get_context()->pfc.bgw_fetch_data[0];
                    // ppu_get_context()->pfc.bgw_fetch_data[0] = (uint8_t)(signed_int + 128);
                    // ppu_get_context()->pfc.bgw_fetch_data[0] += 28;
                }

            }
            ppu_get_context()->pfc.cur_fetch_state = FS_DATA0;
            ppu_get_context()->pfc.fetch_x += 8;
        } break;
        case FS_DATA0: {
            ppu_get_context()->pfc.bgw_fetch_data[1] = BusRead(bus, (LCDC_BGW_DATA_AREA + (ppu_get_context()->pfc.bgw_fetch_data[0] * 16) + ppu_get_context()->pfc.tile_y));

            ppu_get_context()->pfc.cur_fetch_state = FS_DATA1;
        } break;
        case FS_DATA1: {
            ppu_get_context()->pfc.bgw_fetch_data[2] = BusRead(bus, (LCDC_BGW_DATA_AREA + (ppu_get_context()->pfc.bgw_fetch_data[0] * 16) + ppu_get_context()->pfc.tile_y + 1));

            ppu_get_context()->pfc.cur_fetch_state = FS_IDLE;
        } break;
        case FS_IDLE: {
            ppu_get_context()->pfc.cur_fetch_state = FS_PUSH;
        } break;
        case FS_PUSH: {
            if (pipeline_fifo_add()) {
                ppu_get_context()->pfc.cur_fetch_state = FS_TILE;
            }
        } break;
    }
}

void pipeline_push_pixel() {
    if (ppu_get_context()->pfc.pixel_fifo.size > 8) {
        uint32_t pixel_data = pixel_fifo_pop();

        if (ppu_get_context()->pfc.line_x >= (lcd_get_context()->scroll_x % 8)) {
            ppu_get_context()->video_buffer[ppu_get_context()->pfc.pushed_x + (lcd_get_context()->ly * XRES)] = pixel_data;

            ppu_get_context()->pfc.pushed_x++;
        }

        ppu_get_context()->pfc.line_x++;
    }
}

void pipeline_process(Bus *bus) {
    ppu_get_context()->pfc.map_y = (lcd_get_context()->ly + lcd_get_context()->scroll_y) % 256;
    ppu_get_context()->pfc.map_x = (ppu_get_context()->pfc.fetch_x + lcd_get_context()->scroll_x) % 256;


    ppu_get_context()->pfc.tile_y = ((lcd_get_context()->ly + lcd_get_context()->scroll_y) % 8) * 2;

    if (!(ppu_get_context()->line_ticks & 1)) {
        pipeline_fetch(bus);
    }

    pipeline_push_pixel();

}

void pipeline_fifo_reset() {
    while(ppu_get_context()->pfc.pixel_fifo.size) {
        pixel_fifo_pop();
    }

    ppu_get_context()->pfc.pixel_fifo.head = 0;
}